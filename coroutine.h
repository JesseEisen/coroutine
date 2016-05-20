#ifndef __COROUTINE_H__
#define __COROUTINE_H__

#include <ucontext.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define STACK_SIZE  1024*128

struct schedule;

/*define a function pointer for makecontext*/
typedef void (*Fun)(struct schedule *,void *);

/*state for coroutine*/
enum co_state{FREE,RUNNABLE,RUNNING,SUSPEND};

/*the struct for save the coroutine base info*/
typedef struct cothread{
	ucontext_t ctx;
	Fun func;
	int state;
	void *arg;
	char stack[STACK_SIZE];
}cothread_t;

typedef struct schedule{
	ucontext_t main;
	int nco;
	int cap;
	int isrunning;
	cothread_t **co;
}schedule_t;


#endif 
