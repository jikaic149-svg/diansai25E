/**
 * @file  MODE_TASK_SQUARE.c
 * @brief 矩形循迹任务大脑（1m × 1m，4 个内切圆 90° 转向，恒速）
 *
 * 状态机：
 *   INIT     → 锁定里程基线 → 切 STRAIGHT
 *   STRAIGHT → IR 循迹 + 监控里程 + 5s 丢线 watchdog
 *              里程 ≥ SQUARE_TURN_ANTICIPATE  → 切 ARC_TURN
 *              丢线累计 ≥ 5s                   → 切 FINAL_BRAKE
 *   ARC_TURN → 串级 PI（目标 = 入弯航向 + 90°）
 *              |yaw 误差| < 2°  → corner++，下一边或 FINAL_BRAKE
 *   FINAL_BRAKE → 等 MODE_BRAKE 停稳 → DONE
 *   DONE     → 输出 0
 *
 * 备注：丢线 watchdog 仅在 STRAIGHT 阶段计数；ARC 阶段红外本来就脱线，不计。
 */
#include "MODE_TASK_SQUARE.h"
#include "Mode/MODE_TRACKING.h"
#include "Mode/MODE_CURVE.h"
#include "Mode/mode_manager.h"
#include "Application/broker/broker.h"
#include "config.h"
#include "extern.h"
#include <math.h>

typedef enum {
    SQ_INIT = 0,
    SQ_STRAIGHT,
    SQ_ARC_TURN,
    SQ_FINAL_BRAKE,
    SQ_DONE
} SquareState_e;

/* 5ms 控制环 1000 个 tick = 5s */
#define LOST_TICK_LIMIT  ((uint16_t)(SQUARE_LOST_LINE_TIMEOUT / 5))

static SquareState_e s_state          = SQ_INIT;
static uint8_t       s_corner_count   = 0;
static float         s_last_dist      = 0.0f;
static float         s_dist_in_leg    = 0.0f;
static float         s_progress_next  = 0.0f;
static float         s_target_yaw     = 0.0f;
static float         s_arc_target_yaw = 0.0f;
static uint16_t      s_lost_tick      = 0;
static float         s_current_speed  = 0.0f;

static inline float current_distance(void)
{
    float dl = Broker_Subscribe_Float(TOPIC_DISTANCE_LEFT);
    float dr = Broker_Subscribe_Float(TOPIC_DISTANCE_RIGHT);
    return (dl + dr) * 0.5f;
}

static inline float wrap_180(float ang)
{
    while (ang >  180.0f) ang -= 360.0f;
    while (ang < -180.0f) ang += 360.0f;
    return ang;
}

void TaskSquare_Init(void)
{
    s_state          = SQ_INIT;
    s_corner_count   = 0;
    s_last_dist      = 0.0f;
    s_dist_in_leg    = 0.0f;
    s_progress_next  = 0.0f;
    s_target_yaw     = 0.0f;
    s_arc_target_yaw = 0.0f;
    s_lost_tick      = 0;
    s_current_speed  = 0.0f;

    Broker_Publish_Float(TOPIC_TARGET_SPEED_LEFT,  0.0f);
    Broker_Publish_Float(TOPIC_TARGET_SPEED_RIGHT, 0.0f);
}

void TaskSquare_Exec(void)
{
    const float yaw       = Broker_Subscribe_Float(TOPIC_JY61P_YAW);
    const float track_err = Broker_Subscribe_Float(TOPIC_TRACK_ERROR);
    const float dist      = current_distance();
    float ds = 0.0f;

    if (s_state == SQ_INIT) {
        s_last_dist   = dist;
        s_dist_in_leg = 0.0f;
        s_target_yaw  = yaw; // 将当前车头朝向视为第一条边的绝对基准
        s_lost_tick   = 0;
        MODE_Tracking_Init();
        s_state = SQ_STRAIGHT;
        Broker_Publish_Float(TOPIC_TARGET_SPEED_LEFT,  0.0f);
        Broker_Publish_Float(TOPIC_TARGET_SPEED_RIGHT, 0.0f);
        return;
    }

    ds = dist - s_last_dist;
    s_last_dist = dist;

    /* 核心创新：矢量投影里程计！
     * 不论车头怎么晃，永远只累加在当前标准边方向上的有效投影距离。*/
    float yaw_err_curr = wrap_180(yaw - s_target_yaw);
    s_dist_in_leg += ds * cosf(yaw_err_curr * 3.1415926f / 180.0f);

    /* 在弯道中，同时也要累加在“下一条边”方向上的投影（即横向切入距离） */
    if (s_state == SQ_ARC_TURN) {
        float yaw_err_next = wrap_180(yaw - s_arc_target_yaw);
        s_progress_next += ds * cosf(yaw_err_next * 3.1415926f / 180.0f);
    }

    switch (s_state) {
        case SQ_STRAIGHT: {
            /* 5s 丢线 watchdog（只在直线段计数，弧线本来就脱线） */
            if (track_err == 999.0f) {
                if (++s_lost_tick >= LOST_TICK_LIMIT) {
                    s_state = SQ_FINAL_BRAKE;
                    return;
                }
            } else {
                s_lost_tick = 0;
            }

            /* 严格根据矢量投影里程切入弯道 */
            if (s_dist_in_leg >= SQUARE_TURN_ANTICIPATE) {
                s_arc_target_yaw = wrap_180(s_target_yaw + 90.0f); // 绝对理想 90 度
                s_progress_next  = 0.0f; // 重置下一条边的累计里程
                MODE_Curve_Init();
                s_state = SQ_ARC_TURN;
                /* 本帧先按当前速度直行，下帧 ARC_TURN 接管 */
                Broker_Publish_Float(TOPIC_TARGET_SPEED_LEFT,  s_current_speed);
                Broker_Publish_Float(TOPIC_TARGET_SPEED_RIGHT, s_current_speed);
                return;
            }

            /* 速度斜坡上升 */
            if (s_current_speed < SQUARE_BASE_SPEED) {
                s_current_speed += SQUARE_RAMP_ACCEL * 0.005f;
                if (s_current_speed > SQUARE_BASE_SPEED) s_current_speed = SQUARE_BASE_SPEED;
            }

            /* 正常 IR 循迹（按斜坡速度） */
            MODE_Tracking_Exec(s_current_speed, track_err);
            break;
        }

        case SQ_ARC_TURN: {
            const float yaw_err = wrap_180(s_arc_target_yaw - yaw);

            /* 转弯结束条件：剩余误差 < 5° 且看到黑线，或者误差足够小 
             * 收紧到 5 度，强制要求车身几乎完全回正后再切回直线循迹，消除“甩尾”*/
            bool catch_line = false;
            if (fabsf(yaw_err) < 5.0f) {
                if (track_err != 999.0f) {
                    catch_line = true;
                }
            }
            if (fabsf(yaw_err) < SQUARE_TURN_DONE_DEG) {
                catch_line = true;
            }

            if (catch_line) {
                s_corner_count++;
                if (s_corner_count >= SQUARE_TOTAL_CORNERS) {
                    s_state = SQ_FINAL_BRAKE;
                    return;
                }
                /* 状态切换到下一条边：
                 * 抛弃固定的 132mm 经验补偿！
                 * 直接将本弯道中积分出的“下一边投影距离”无缝继承给新的 dist_in_leg。
                 * 哪怕因为抓线早了导致实际切入没那么深，矢量积分出的值也绝对是真理！*/
                s_target_yaw  = s_arc_target_yaw;
                s_dist_in_leg = s_progress_next;
                s_lost_tick   = 0;
                MODE_Tracking_Init();
                s_state = SQ_STRAIGHT;
                /* 本帧维持当前速度直行 */
                Broker_Publish_Float(TOPIC_TARGET_SPEED_LEFT,  s_current_speed);
                Broker_Publish_Float(TOPIC_TARGET_SPEED_RIGHT, s_current_speed);
                return;
            }

            /* 速度斜坡上升（防止在弯道内突变） */
            if (s_current_speed < SQUARE_BASE_SPEED) {
                s_current_speed += SQUARE_RAMP_ACCEL * 0.005f;
                if (s_current_speed > SQUARE_BASE_SPEED) s_current_speed = SQUARE_BASE_SPEED;
            }

            /* 串级 PI 弧线行进 */
            MODE_Curve_Exec(s_current_speed, s_arc_target_yaw, yaw);
            break;
        }

        case SQ_FINAL_BRAKE:
            /* 保持直行循迹，同时以斜坡平滑减速至 0 */
            if (s_current_speed > 0.0f) {
                s_current_speed -= SQUARE_RAMP_ACCEL * 0.005f;
                if (s_current_speed <= 0.0f) {
                    s_current_speed = 0.0f;
                    Broker_Publish_Float(TOPIC_TARGET_SPEED_LEFT,  0.0f);
                    Broker_Publish_Float(TOPIC_TARGET_SPEED_RIGHT, 0.0f);
                    Mode_SetIdle(); /* 彻底停稳后切回 Idle */
                } else {
                    /* 如果没丢线，继续循迹；如果丢线了，Tracking_Exec 会默认直行，不影响减速 */
                    MODE_Tracking_Exec(s_current_speed, track_err);
                }
            } else {
                Broker_Publish_Float(TOPIC_TARGET_SPEED_LEFT,  0.0f);
                Broker_Publish_Float(TOPIC_TARGET_SPEED_RIGHT, 0.0f);
            }
            break;

        case SQ_DONE:
        default:
            Broker_Publish_Float(TOPIC_TARGET_SPEED_LEFT,  0.0f);
            Broker_Publish_Float(TOPIC_TARGET_SPEED_RIGHT, 0.0f);
            break;
    }
}

uint8_t TaskSquare_IsDone(void)
{
    return (s_state == SQ_DONE || s_state == SQ_FINAL_BRAKE) ? 1 : 0;
}
