/* 
 * stoplight.c
 *
 * 31-1-2003 : GWA : Stub functions created for CS161 Asst1.
 *
 * NB: You can use any synchronization primitives available to solve
 * the stoplight problem in this file.
 */


/*
 * 
 * Includes
 *
 */

#include <types.h>
#include <lib.h>
#include <test.h>
#include <thread.h>
#include <synch.h>

/*
 *
 * Constants
 *
 */

/*
 * Number of cars created.
 */

#define NCARS 20

struct lock *NW;
struct lock *NE;
struct lock *SW;
struct lock *SE;
struct lock *print;
 int howmanyended=0;


/*
 *
 * Function Definitions
 *
 */

static const char *directions[] = { "N", "E", "S", "W" };

static const char *msgs[] = {
        "approaching:",
        "region1:    ",
        "region2:    ",
        "region3:    ",
        "leaving:    "
};

/* use these constants for the first parameter of message */
enum { APPROACHING, REGION1, REGION2, REGION3, LEAVING };

static void
message(int msg_nr, int carnumber, int cardirection, int destdirection)
{
  kprintf("%s car = %2d, direction = %s, destination = %s\n", msgs[msg_nr], carnumber, directions[cardirection], directions[destdirection]);
}
 
/*
 * gostraight()
 *
 * Arguments:
 *      unsigned long cardirection: the direction from which the car
 *              approaches the intersection.
 *      unsigned long carnumber: the car id number for printing purposes.
 *
 * Returns:
 *      nothing.
 *
 * Notes:
 *      This function should implement passing straight through the
 *      intersection from any direction.
 *      Write and comment this function.
 */

static void gostraight(unsigned long cardirection, unsigned long carnumber) {
  if (cardirection==0) {
    int i = 0;
    int destination=2;
    int spl;
    
    lock_acquire2(NW,SW);

    
    message (i,carnumber,cardirection,destination);
    

    i=1;
    
    message (i,carnumber,cardirection,destination);
    
    
    i=2;
    
    message (i,carnumber,cardirection,destination);


    i=4;
    
    message (i,carnumber,cardirection,destination);
    

    lock_release2(NW,SW);

	 }
	
  else if (cardirection==1) {
    int i = 0;
    int destination=3;
    int spl;

    lock_acquire2(NE,NW);

    
    message (i,carnumber,cardirection,destination);
    

    i=1;
    
    message (i,carnumber,cardirection,destination);
    

    i=2;
    
    message (i,carnumber,cardirection,destination);
    

    i=4;
    
    message (i,carnumber,cardirection,destination);
    

    lock_release2(NE,NW);
	
	}

	else if (cardirection==2) {
    int spl;
    int i = 0;
    int destination=0;

    lock_acquire2(SE,NE);

    
    message (i,carnumber,cardirection,destination);
    

    i=1;
    
    message (i,carnumber,cardirection,destination);


    i=2;
    
    message (i,carnumber,cardirection,destination);
    

    i=4;
    
    message (i,carnumber,cardirection,destination);
    

    lock_release2(SE,NE);

	
	}

	else if (cardirection==3) {
    int spl;
    int i = 0;
    int destination=1;

    lock_acquire2(SW,SE);

    
    message (i,carnumber,cardirection,destination);
    

    i=1;
    
    message (i,carnumber,cardirection,destination);
    

    i=2;
    
    message (i,carnumber,cardirection,destination);
    

    i=4;
    
    message (i,carnumber,cardirection,destination);
    
    lock_release2(SW,SE);
	}
     
}


/*
 * turnleft()
 *
 * Arguments:
 *      unsigned long cardirection: the direction from which the car
 *              approaches the intersection.
 *      unsigned long carnumber: the car id number for printing purposes.
 *
 * Returns:
 *      nothing.
 *
 * Notes:
 *      This function should implement making a left turn through the 
 *      intersection from any direction.
 *      Write and comment this function.
 */

static void turnleft(unsigned long cardirection, unsigned long carnumber) {
	
  if (cardirection==0) {
    int spl;
    int i = 0;
    int destination=1;
    
    lock_acquire3(NW,SW,SE);
    
    
    message (i,carnumber,cardirection,destination);
    

    i=1;
    
    message (i,carnumber,cardirection,destination);
    

    i=2;

    message (i,carnumber,cardirection,destination);
    

    i=3;
    
    message (i,carnumber,cardirection,destination);
    

    i=4;
    ;
    message (i,carnumber,cardirection,destination);
    
    
    lock_release3(NW,SW,SE);
	
	}
	
  else if (cardirection==1) {
    int spl;
    int i = 0;
    int destination=2;

    lock_acquire3(NE,NW,SW);
    
    message (i,carnumber,cardirection,destination);
    

    i=1;
    
    message (i,carnumber,cardirection,destination);
    

    i=2;
    
    message (i,carnumber,cardirection,destination);
    

    i=3;
    
    message (i,carnumber,cardirection,destination);
    

    i=4;
    
    message (i,carnumber,cardirection,destination);
    
    
    lock_release3(NE,NW,SW);

	}

  else if (cardirection==2) {
    int spl;
    int i = 0;
    int destination=3;

    lock_acquire3(SE,NE,NW);
    
    
    message (i,carnumber,cardirection,destination);
    

    i=1;

    message (i,carnumber,cardirection,destination);
    

    i=2;
    
    message (i,carnumber,cardirection,destination);	
    

    i=3;
    
    message (i,carnumber,cardirection,destination);
    

    i=4;
    ;
    message (i,carnumber,cardirection,destination);
    

    
    lock_release3(SE,NE,NW);
    
	
	}

	else if (cardirection==3) {
    int i = 0;
    int destination=0;
    int spl;

    lock_acquire3(SW,SE,NE);
    
    
    message (i,carnumber,cardirection,destination);
    

    i=1;
    
    message (i,carnumber,cardirection,destination);
    

    i=2;
    
    message (i,carnumber,cardirection,destination);
    

    i=3;
    
    message (i,carnumber,cardirection,destination);


    i=4;
    
    message (i,carnumber,cardirection,destination);
    
    
    lock_release3(SW,SE,NE);
	
	}

	
	
	
}


/*
 * turnright()
 *
 * Arguments:
 *      unsigned long cardirection: the direction from which the car
 *              approaches the intersection.
 *      unsigned long carnumber: the car id number for printing purposes.
 *
 * Returns:
 *      nothing.
 *
 * Notes:
 *      This function should implement making a right turn through the 
 *      intersection from any direction.
 *      Write and comment this function.
 */

static void turnright(unsigned long cardirection, unsigned long carnumber) {
      
	if (cardirection==0) {
    int j = 0;
    int destination=3;
    int spl;

    lock_acquire(NW);
    
    message (j,carnumber,cardirection,destination);
    

    j=1;
    
    message (j,carnumber,cardirection,destination);
      

    j=4;
    
    message (j,carnumber,cardirection,destination);
    

    lock_release(NW);
	}

	else if (cardirection==1) {
    int g = 0;
    int destination=0;
    int spl;

    lock_acquire(NE);

    
    message (g,carnumber,cardirection,destination);
    

    g=1;
    
    message (g,carnumber,cardirection,destination);
    

    g=4;
    
    message (g,carnumber,cardirection,destination);


    lock_release(NE);
	}

	else if (cardirection==2) {
    int spl;
    int k = 0;
    int destination=1;

    lock_acquire(SE);

    
    message (k,carnumber,cardirection,destination);
    

    k=1;
    
    message (k,carnumber,cardirection,destination);
    

    k=4;
    
    message (k,carnumber,cardirection,destination);
    

    lock_release(SE);
	}

	else if (cardirection==3) {
    int i = 0;
    int destination=2;
    int spl;

    lock_acquire(SW);

    
    message (i,carnumber,cardirection,destination);
    

    i=1;
    
    message (i,carnumber,cardirection,destination);
    

    i=4;
    
    message (i,carnumber,cardirection,destination);


    lock_release(SW);
	}

	
     
	

}


/*
 * approachintersection()
 *
 * Arguments: 
 *      void * unusedpointer: currently unused.
 *      unsigned long carnumber: holds car id number.
 *
 * Returns:
 *      nothing.
 *
 * Notes:
 *      Change this function as necessary to implement your solution. These
 *      threads are created by createcars().  Each one must choose a direction
 *      randomly, approach the intersection, choose a turn randomly, and then
 *      complete that turn.  The code to choose a direction randomly is
 *      provided, the rest is left to you to implement.  Making a turn
 *      or going straight should be done by calling one of the functions
 *      above.
 */
 
static void approachintersection(void * unusedpointer, unsigned long carnumber)
{

        int cardirection;
	int turndirection;

        /*
         * Avoid unused variable and function warnings.
         */

        (void) unusedpointer;
        (void) carnumber;
	(void) gostraight;
	(void) turnleft;
	(void) turnright;

        /*
         * cardirection is set randomly.
         */

        cardirection = random() % 4;
	turndirection = random() % 3;
	int i=0;


	if (turndirection==0)
		{
		//kprintf("carnumber is %d cardirection is %d turndirection is %d\n",carnumber,cardirection,turndirection );
		turnleft(cardirection,carnumber);
		}

	else if (turndirection==1)
		{
		//kprintf("carnumber is %d cardirection is %d turndirection is %d\n",carnumber,cardirection,turndirection );
		gostraight(cardirection,carnumber);
		}

	else if (turndirection==2)
		{
		//kprintf("carnumber is %d cardirection is %d turndirection is %d\n",carnumber,cardirection,turndirection );
		turnright(cardirection,carnumber);
		}
int spl;
spl=splhigh();
howmanyended++;
splx(spl);

}


/*
 * createcars()
 *
 * Arguments:
 *      int nargs: unused.
 *      char ** args: unused.
 *
 * Returns:
 *      0 on success.
 *
 * Notes:
 *      Driver code to start up the approachintersection() threads.  You are
 *      free to modiy this code as necessary for your solution.
 */

int
createcars(int nargs,
           char ** args)
{
        int index, error;

        /*
         * Avoid unused variable warnings.
         */

        (void) nargs;
        (void) args;

        /*
         * Start NCARS approachintersection() threads
         */
	NW=lock_create("NW");
	NE=lock_create("NE");
	SW=lock_create("SW");
	SE=lock_create("SE");
	
	

        for (index = 0; index < NCARS; index++) {

                error = thread_fork("approachintersection thread",
                                    NULL,
                                    index,
                                    approachintersection,
                                    NULL
                                    );

                /*
                 * panic() on error.
                 */

                if (error) {
                        
                        panic("approachintersection: thread_fork failed: %s\n",
                              strerror(error)
                              );
                }
        }
while (howmanyended<20)
{
}

        return 0;
}
