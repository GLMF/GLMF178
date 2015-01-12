/*
Le programme suivant est fourni a titre d'exemple pour illustrer un article.
Les instructions superflues telles que le test des codes de retour des
fonctions ont ete volontairement expurgees pour simplifier la lecture.

Rachid Koucha
*/
#include <errno.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/syscall.h>
#include <linux/futex.h>
#include <sched.h>
#include <sched.h>
#include <stdlib.h>
#include <limits.h>

#define futex_wait_to(addr, val, to) \
           syscall(SYS_futex, addr, FUTEX_WAIT_PRIVATE, val, to)

void mysleep(unsigned int sec)
{
int             futex_var = 0;
struct timespec timeout;

  timeout.tv_sec = sec;
  timeout.tv_nsec = 0;

  futex_wait_to(&futex_var, 0, &timeout);
}

int nb_thread = 5;

#define futex_wait(addr, val) \
           syscall(SYS_futex, addr, FUTEX_WAIT, val, NULL)

#define futex_wakeup(addr, nb) \
           syscall(SYS_futex, addr, FUTEX_WAKE, nb)

int futex1;
int futex2;

void *thd_main(void *p)
{
int                *idx = (int *)p;
struct sched_param  param;

  param.sched_priority = nb_thread - *idx;
  pthread_setschedparam(pthread_self(), SCHED_RR, &param);

  printf("Thread#%d attend sur le futex1 (prio = %d)\n", *idx, param.sched_priority);

  futex_wait(&futex1, 0);

  printf("Thread#%d reveille\n", *idx);

  return NULL;
}

#define futex_cmp_requeue(addr1, nb_wake, nb_move, addr2, val1) \
    syscall(SYS_futex, addr1, FUTEX_CMP_REQUEUE, nb_wake, nb_move, addr2, val1)

int main(int ac, char *av[])
{
pthread_t *tid;
int        i;
int       *idx;
int        rc;

  printf("Thread#0 demarre\n");

  idx = (int *)malloc(nb_thread * sizeof(int));
  tid = (pthread_t *)malloc(nb_thread * sizeof(pthread_t)); 

  tid[0] = pthread_self();

  for (i = 1; i < nb_thread; i ++)
  {
    idx[i] = i;
    pthread_create(&(tid[i]), NULL, thd_main, (void *)&(idx[i]));
  }

  // Attente pour que les threads soient sur le futex1
  mysleep(1);

  // Reveil de thread#1
  // Transfert de thread#2 et thread#3 sur futex2
  // Thread#4 reste en attente sur futex1
  rc = futex_cmp_requeue(&futex1, 1, 2, &futex2, 0); 
  printf("Thread#0 : rc = %d\n", rc);

  // Attente de la terminaison du thread#1
  pthread_join(tid[1], NULL);

  // Reveil des thread#2 et thread#3 en attente sur le futex2
  rc = futex_wakeup(&futex2, INT_MAX);
  printf("Thread#0 : rc = %d\n", rc);

  // Attente de la terminaison du thread#2 et thread#3
  pthread_join(tid[2], NULL);
  pthread_join(tid[3], NULL);

  // Reveil du thread#4 en attente sur le futex1
  rc = futex_wakeup(&futex1, INT_MAX);
  printf("Thread#0 : rc = %d\n", rc);

  // Attente de la terminaison du thread#4
  pthread_join(tid[4], NULL);

  free(idx);
  free(tid);
  
  return 0;
}
