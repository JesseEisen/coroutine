#include "coroutine.h"


void
func(schedule_t *s, void *arg)
{
	int i;
	int *temp =(int *)arg;
	
	for(i = 0; i< 5; i++){
		printf("coroutine %d: %d\n",cothread_current(s), *temp + i);
		cothread_yield(s);
	}
	
}




void
schedule_test()
{
	schedule_t *s = cothread_new();
	int arg1 = 100;
	int arg2 = 1;

	int id1 = cothread_create(s, func, &arg1);
	int id2 = cothread_create(s, func, &arg2);
	puts("main start");
	while(cothread_status(s,id1) && cothread_status(s,id2)){
		cothread_resume(s,id1);
		cothread_resume(s,id2);
	}
	puts("main finished");
	cothread_close(s);
}


int
main(void)
{
	schedule_test();
}
