#define INCL_DOS
#define INCL_DOSPROCESS
#include <stdlib.h>
#include <stdio.h>
#include <os2.h>
 
extern USHORT APIENTRY DosQProcStatus( PVOID pBuf, USHORT cbBuf );
void Put_name( char *name );
void Put_state( unsigned state );
 
BYTE bBuf[0x2000];
 
typedef union _ps_info PS_info;
 
struct _reference_info
  { /* type 0 */
    unsigned       TYPE;
    PS_info        *NEXT;
    unsigned       PID;
    unsigned       PARENT_PID;
    unsigned       SGID;
    unsigned       MODULE_HANDLE;
    unsigned       *MODULE_REFS;     /* points to modules handle numbers */
    unsigned       MODULE_REF_COUNT;
    unsigned       *SHARED_MEM_REFS; /* points to shared mem handle numbers */
    unsigned       SHARED_MEM_COUNT;
    unsigned       *SEMAPHORE_REFS;  /* points to semaphore handle numbers */
    unsigned       SEMAPHORE_COUNT;
  };
 
struct _thread_info
  { /* type 1 */
    unsigned       TYPE;
    PS_info        *NEXT;
    unsigned       PD_INDEX;
    unsigned       PID;
    unsigned       TID;
    unsigned       PRIORITY;
    unsigned       STATE;
    unsigned long  BLOCK_ID;
  };
 
struct _module_info
  { /* type 2 */
    unsigned       TYPE;
    PS_info        *NEXT;
    unsigned       HANDLE;
    unsigned       NUM_DEPENDENCIES;      /* number of imported modules */
    unsigned       DEPENDENCIES_PTR;
    char           *MODULE_NAME_PTR;
    unsigned       DEPENDENT_HANDLE_LIST; /* imported handle list */
  };
 
struct _semaphore_info
  { /* type 3 */
    unsigned       TYPE;
    PS_info        *NEXT;
    unsigned       FLAGS;
    unsigned       REF;
    unsigned       HANDLE;
    char           SEM_NAME;
  };
 
struct _shared_mem_info
  { /* type 4 */
    unsigned       TYPE;
    PS_info        *NEXT;
    unsigned       HANDLE;
    unsigned       SELECTOR;
    unsigned       REF;
    char           SEG_NAME;
  };
 
typedef union _ps_info
  {
    struct _reference_info   t0;
    struct _thread_info      t1;
    struct _module_info      t2;
    struct _semaphore_info   t3;
    struct _shared_mem_info  t4;
  } PS_info;
 
 
 
 
void main( int    argc,
           char   *argv[] )
  {
    PIDINFO   pid;
    USHORT    type;
    PS_info   *info;
    unsigned  *pw;
    unsigned  i;
 
 
    DosGetPID( &pid );
    printf( "Process ID  = 0x%04X\n", pid.pid );
    printf( "Process TID = 0x%04X\n", pid.tid );
    printf( "Parent ID   = 0x%04X\n", pid.pidParent );
    if( DosQProcStatus(bBuf, sizeof(bBuf)) ) exit( 1 );
 
    printf( "Buffer address is %p:\n\n", bBuf );
    info = (PS_info *) bBuf;
    for(;;)
      {
        type = info->t0.TYPE;
        if( type == 0xFFFF ) break;
 
   /*     printf( "Info @ %04X : Type = %u, Next = %04X\n",
                 info, type, info->t0.NEXT );  */
 
        switch( type )
          {
          case 0:
            printf( "Reference Information:  " );
            printf( "PID=%04X PARENT_PID=%04X SGID=%04X MODULE_HANDLE=%04X\n",
                    info->t0.PID, info->t0.PARENT_PID, info->t0.SGID, info->t0.MODULE_HANDLE );
            pw = info->t0.MODULE_REFS;
            printf( "Module reference handles:" );
            for( i=0; i<info->t0.MODULE_REF_COUNT; ++i )
                printf( " %04X", pw[i] );
            pw = info->t0.SHARED_MEM_REFS;
            printf( "\nShared memory handles:" );
            for( i=0; i<info->t0.SHARED_MEM_COUNT; ++i )
                printf( " %04X", pw[i] );
            pw = info->t0.SEMAPHORE_REFS;
            printf( "\nSemaphore handles:" );
            for( i=0; i<info->t0.SEMAPHORE_COUNT; ++i )
                printf( " %04X", pw[i] );
            break;
 
          case 1:
            printf( "Thread Information:  " );
            printf( "PD_INDEX=%04X PID=%04X TID=%04X PRIORITY=%04X BLOCK_ID=%08lX STATE=",
                    info->t1.PD_INDEX, info->t1.PID, info->t1.TID,
                    info->t1.PRIORITY, info->t1.BLOCK_ID );
            Put_state( info->t1.STATE );
            break;
 
          case 2:
            printf( "Module  Information:  " );
            printf( "HANDLE=%04X NUM_DEPENDENCIES=%04X\n",
                    info->t2.HANDLE, info->t2.NUM_DEPENDENCIES );
            pw = &info->t2.DEPENDENT_HANDLE_LIST;
            printf( "Dependent handles:" );
            for( i=0; i<info->t2.NUM_DEPENDENCIES; ++i )
                printf( " %04X", pw[i] );
            printf( "\nModule Name: " );
            Put_name( info->t2.MODULE_NAME_PTR );
            break;
 
          case 3:
            printf( "System Semaphore Information:  " );
            printf( "FLAGS=%04X REF=%04X HANDLE=%04X\n",
                    info->t3.FLAGS, info->t3.REF, info->t3.HANDLE );
            printf( "Semaphore Name: "  );
            Put_name( &info->t3.SEM_NAME );
            break;
 
          case 4:
            printf( "Named Shared Segment Information:  " );
            printf( "HANDLE=%04X SELECTOR=%04X REF=%04X\n",
                    info->t4.HANDLE, info->t4.SELECTOR, info->t4.REF );
            printf( "Shared Segment Name: " );
            Put_name( &info->t4.SEG_NAME );
            break;
 
          default:
            printf( "Unknown Information:\n" );
            break;
          }
        printf( "\n\n" );
 
        info = info->t0.NEXT;
      }
 
    printf( "End of Chain\n" );
  }
 
 
 
void Put_name( char  *name )
  {
    unsigned       i;
 
    for( i=0; i<64; ++i )
      {
        if( *name == '\0' ) break;
        if( *name >= ' ' && *name < 0x7F )
            putchar( *name );
        ++name;
      }
  }
 
 
 
void  Put_state( unsigned state )
  {
 
    switch( state )
      {
        case 0      : printf( "start   " ); break;
        case 1      : printf( "zombie  " ); break;
        case 2      : printf( "ready   " ); break;
        case 3      : printf( "blocked " ); break;
        case 4      : printf( "frozen  " ); break;
        case 5      : printf( "critsec " ); break;
        case 6      : printf( "backgnd " ); break;
        case 7      : printf( "boost   " ); break;
        default     : printf( "0x%02X    ", state ); break;
      }
  }
