/* Copyright(C) 2018 Hex Five Security, Inc. - All Rights Reserved */

#include <platform.h> // RTC_FREQ
#include "owi_task.h"

typedef enum{
	START_REQUEST,
	STARTED,
	STOP_REQUEST,
	STOPPED,
	FOLD,
	UNFOLD
} state_enum;

typedef enum{
	STOP 				= 0x000000,
	GRIP_CLOSE			= 0x000001,
	GRIP_OPEN 			= 0x000002,
	WRIST_UP 			= 0x000004,
	WRIST_DOWN 			= 0x000008,
	ELBOW_UP   			= 0x000010,
	ELBOW_DOWN			= 0x000020,
	SHOULDER_UP   		= 0x000040,
	SHOULDER_DOWN		= 0x000080,
	BASE_CLOCKWISE 	  	= 0x000100,
	BASE_COUNTERCLOCK 	= 0x000200,
	LIGHT_ON  			= 0x010000,
	ARM_UP    = 0x000008 | 0x000010 | 0x000040, 		   // wrist down + elbow up   + shoulder up
	ARM_DOWN  = 0x000004 | 0x000020 | 0x000080 | 0x000100, // wrist up   + elbow down + shoulder down + base clockwise
} cmd;

struct sequence_step{
	uint32_t command;
	int duration_ms;
};

#define T_STOP 	1000
#define T_GRIP 	1400
#define T_WRIST 1900
#define T_ARM  	1600
#define T_BASE	3000
static struct sequence_step main_sequence[] = {
	{ .command = STOP, 				.duration_ms = T_STOP },
	{ .command = GRIP_CLOSE, 		.duration_ms = T_GRIP },
	{ .command = WRIST_UP, 			.duration_ms = T_WRIST},
	{ .command = ARM_DOWN, 			.duration_ms = T_ARM  },
	{ .command = BASE_CLOCKWISE,	.duration_ms = T_BASE },
	{ .command = GRIP_OPEN, 		.duration_ms = T_GRIP },
	{ .command = STOP, 				.duration_ms = T_STOP },
	{ .command = GRIP_CLOSE, 		.duration_ms = T_GRIP },
	{ .command = BASE_COUNTERCLOCK, .duration_ms = T_BASE+T_ARM -125}, // -100
	{ .command = ARM_UP, 			.duration_ms = T_ARM  },
    { .command = SHOULDER_UP, 		.duration_ms = +50 }, // +75
    { .command = ELBOW_UP, 			.duration_ms = +125 }, // +150
	{ .command = WRIST_DOWN, 		.duration_ms = T_WRIST -300},
	{ .command = GRIP_OPEN, 		.duration_ms = T_GRIP },
};

#define T_FOLD 4500
#define T_FOLD_SHOULDER 800
static struct sequence_step fold_sequence[] = {
	{ .command = STOP, .duration_ms = 0},
	{ .command = GRIP_OPEN, .duration_ms = 100},
	{ .command = ELBOW_DOWN | SHOULDER_UP | WRIST_DOWN, .duration_ms = T_FOLD},
	{ .command = ELBOW_DOWN, .duration_ms = 200},
	{ .command = SHOULDER_UP, .duration_ms = T_FOLD_SHOULDER},
};
static struct sequence_step unfold_sequence[] = {
	{ .command = STOP, .duration_ms = 0},
	{ .command = ELBOW_UP, .duration_ms = 200},
	{ .command = SHOULDER_DOWN, .duration_ms = T_FOLD_SHOULDER + 50},
	{ .command = ELBOW_UP | SHOULDER_DOWN | WRIST_UP, .duration_ms = T_FOLD},
};

static state_enum state = STOPPED;
static int step = -1;
static uint64_t timer = 0;

void owi_task_start_request(void){if (state==STOPPED) state=START_REQUEST;}
void owi_task_stop_request(void){if (state==STARTED) state=STOP_REQUEST;}
void owi_task_fold(void){if (state==STOPPED) state=FOLD; step=0; timer=0;}
void owi_task_unfold(void){if (state==STOPPED) state=UNFOLD; step=0; timer=0;}

int32_t owi_task_run(const uint64_t time){

	int32_t cmd = -1;

	switch(state){

		case START_REQUEST :
			step = 0; timer = 0; state=STARTED;
		    break;

		case (STARTED) :
		case (STOP_REQUEST) :
			if (time > timer){
				step = (step+1) % (sizeof(main_sequence)/sizeof(main_sequence[0]));
				timer = time + main_sequence[step].duration_ms * RTC_FREQ/1000;
				cmd = main_sequence[step].command;
				cmd = (state==STOP_REQUEST ? cmd & ~(1UL<<16) : cmd | (1UL<<16));
			}
			if (state==STOP_REQUEST && step==0)	state = STOPPED;
			break;

		case (FOLD) :
			if (time > timer){
				step = (step+1) % (sizeof(fold_sequence)/sizeof(fold_sequence[0]));
				timer = time + fold_sequence[step].duration_ms * RTC_FREQ/1000;
				cmd = fold_sequence[step].command;
				if (step==0) state = STOPPED;
			} break;

		case (UNFOLD) :
			if (time > timer){
				step = (step+1) % (sizeof(unfold_sequence)/sizeof(unfold_sequence[0]));
				timer = time + unfold_sequence[step].duration_ms * RTC_FREQ/1000;
				cmd = unfold_sequence[step].command;
				if (step==0) state = STOPPED;
			} break;

		case STOPPED :
			break;

		default: break;

	}

	return cmd;

}

