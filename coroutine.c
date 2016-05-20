#include "coroutine.h"

#define THREAD_NUMS  1024;


/** 
 * This function is used to init a schedule
 * */

schedule_t *
cothread_new(void)
{
	schedule_t *sc = malloc(sizeof(*sc));
	sc->nco = 0;
	sc->cap = THREAD_NUMS;
	sc->isrunning = -1;
	sc->co= malloc(sizeof(struct cothread *) * sc->cap);
	memset(sc->co,0, sizeof(struct cothread *) * sc->cap);

	return sc;
}

static cothread_t *
_co_new(schedule_t *sc, Fun func, void *arg)
{
	cothread_t *co = malloc(sizeof(*co));
	co->func = func;
	co->arg = arg;
	co->state= RUNNABLE;
	memset(co->stack,0,STACK_SIZE);

	return co;
}


/**
 * This is a function for create a new routine
 * /para  sc   the schedule 
 * /para  func user's own function
 * /para  arg  the arg will put to the function
 * */
int
cothread_create(schedule_t *sc, Fun func, void *arg)
{
	cothread_t *co = _co_new(sc,func,arg);

	int i;
	for(i = 0; i<sc->cap; i++)
	{
		int id = (i+sc->nco)%sc->cap;
		if(sc->co[id] == NULL)
		{
			sc->co[id] = co;
			++sc->nco;
			return id;
		}
	}

	assert(0);
	return -1;
}

static void
mainfunc(schedule_t *sc)
{
	int id = sc->isrunning;

	cothread_t *co = sc->co[id];
	co->func(sc,co->arg);
	sc->co[id] = NULL;
	--sc->nco;
	sc->isrunning = -1;
}


/**
 * This function is used to resume the 
 * the routine that been yielded.
 * */
void
cothread_resume(schedule_t *sc,int id)
{
	assert(sc->isrunning == -1);
	assert(id >= 0 && id < sc->cap);

	cothread_t *co = sc->co[id];
	if(co == NULL)
		return;

	switch(co->state){
		case RUNNABLE:
			getcontext(&co->ctx);
			co->ctx.uc_stack.ss_sp = co->stack;
			co->ctx.uc_stack.ss_size = STACK_SIZE;
			co->ctx.uc_link = &sc->main;
			sc->isrunning = id;
			co->state = RUNNING;

			makecontext(&co->ctx,(void (*)(void)) mainfunc,1,sc);
			swapcontext(&sc->main,&co->ctx);
			break;
		case SUSPEND:
			sc->isrunning = id;
			co->state = RUNNING;
			swapcontext(&sc->main, &co->ctx);
			break;
		default:
			assert(0);
	}
}

void
cothread_yield(schedule_t *sc)
{
	int id = sc->isrunning;
	assert(id >=0);

	cothread_t *co = sc->co[id];
	co->state = SUSPEND;
	sc->isrunning = -1;
	swapcontext(&co->ctx, &sc->main);
}

int 
cothread_status(schedule_t *sc, int id)
{
	assert(id >=0 && id < sc->cap);
	if(sc->co[id] == NULL)
		return FREE;

	return sc->co[id]->state;
}

int 
cothread_current(schedule_t *sc)
{
	return sc->isrunning;
}

void 
cothread_close(schedule_t *sc)
{
	int i;
	for(i =0; i<sc->cap; i++)
	{
		cothread_t *co = sc->co[i];
		if(co){
			free(co);
		}
	}

	free(sc->co);
	sc->co = NULL;
	free(sc);
}


