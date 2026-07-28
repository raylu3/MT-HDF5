#include <assert.h>
#include <stddef.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>

#include "BPTS.h"

/***************************************************************************
 * 
 * BPTS_alloc_pt_rec_entry_count
 *
 *    Allocate and initalize an instance of BPTS_pt_rec_entry_count_t.
 *
 *                                               JRM -- 4/23/25
 *
 * RETURNS:
 *
 *    Pointer to allocated and initialized instance of
 *    H5TS_pt_rec_entry_count_t, or NULL on failure.
 *
 *
 **************************************************************************/

BPTS_pt_rec_entry_count_t *
BPTS_alloc_pt_rec_entry_count(bool exclusive_lock)
{   
    BPTS_pt_rec_entry_count_t * ret_value = NULL;
    
    ret_value = (BPTS_pt_rec_entry_count_t *)
                malloc(sizeof(BPTS_pt_rec_entry_count_t));
    
    if ( ret_value ) {
        
        ret_value->tag            = BPTS_PT_REC_XS_REC_ENTRY_COUNT_TAG;
        ret_value->exclusive_lock = exclusive_lock;
        ret_value->rec_lock_count = 1;
    }
    
    return(ret_value);

} /* BPTS_alloc_pt_rec_entry_count() */


/***************************************************************************
 * 
 * BPTS_free_pt_rec_entry_count
 *
 *    Discard the supplied instance of BPTS_pt_rec_entry_count_t.
 *
 *                                               JRM -- 4/23/25
 *
 * Returns: Void.
 *
 **************************************************************************/

void
BPTS_free_pt_rec_entry_count(void * target_ptr)
{
    BPTS_pt_rec_entry_count_t * count_ptr;

    count_ptr = (BPTS_pt_rec_entry_count_t *)target_ptr;

    assert(count_ptr);
    assert(count_ptr->tag == BPTS_PT_REC_XS_REC_ENTRY_COUNT_TAG);

    count_ptr->tag = 0;

    free(count_ptr);

    return;

} /* BPTS_free_pt_rec_entry_count() */


/***************************************************************************
 * 
 * BPTS_pt_rec_xs_lock_init
 *
 *    Initialize the supplied instance of BPTS_pt_rec_xs_lock_t.
 *
 *                                            JRM -- 4/23/25
 *
 * Returns: SUCCEED on success, FAIL on failure.
 *
 **************************************************************************/

herr_t
BPTS_pt_rec_xs_lock_init(BPTS_pt_rec_xs_lock_t *xs_lock_ptr, int policy,
                         BPTS_pt_rec_xs_x2s_func_t x2s_func, void * x2s_data)
{
    herr_t ret_value = SUCCEED;

    /* santity checks -- until other policies are implemented,
     * policy must equal BPTS__XS_LOCK_POLICY__FAVOR_EXCLUSIVE_ACCESS.
     */
    if ( ( xs_lock_ptr == NULL ) ||
         ( policy != BPTS__XS_LOCK_POLICY__FAVOR_EXCLUSIVE_ACCESS ) ) {

        ret_value = FAIL;
    }

    if ( ret_value == SUCCEED ) { /* initialized the mutex */

        if ( pthread_mutex_init(&(xs_lock_ptr->mutex), NULL) != 0 ) {

            ret_value = FAIL;
        }
    }

    if ( ret_value == SUCCEED ) { /* initialize the waiting shared cv */

        if ( pthread_cond_init(&(xs_lock_ptr->shared_cv), NULL) != 0 ) {

            ret_value = FAIL;
        }
    }

    if ( ret_value == SUCCEED ) { /* initialize the waiting exclusife cv */

        if ( pthread_cond_init(&(xs_lock_ptr->exclusive_cv), NULL) != 0 ) {

            ret_value = FAIL;
        }
    }


    if ( ret_value == SUCCEED ) { /* initialize the key */

        if ( pthread_key_create(&(xs_lock_ptr->rec_entry_count_key),
                                 BPTS_free_pt_rec_entry_count) != 0 ) {

            ret_value = FAIL;
        }

    }

    if ( ret_value == SUCCEED ) { /* initialized scalar fields */

        xs_lock_ptr->tag = BPTS_PT_REC_XS_LOCK_TAG;

        xs_lock_ptr->policy                                   = policy;
        xs_lock_ptr->waiting_shared_count                     = 0;
        xs_lock_ptr->waiting_exclusive_count                  = 0;
        xs_lock_ptr->active_shared                            = 0;
        xs_lock_ptr->active_exclusive                         = 0;
        xs_lock_ptr->last_lock_exclusive                      = FALSE;
        xs_lock_ptr->x2s_func                                 = x2s_func;
        xs_lock_ptr->x2s_data                                 = x2s_data;

        xs_lock_ptr->stats.exclusive_locks_granted            = 0;
        xs_lock_ptr->stats.exclusive_locks_coerced            = 0;
        xs_lock_ptr->stats.exclusive_locks_released           = 0;
        xs_lock_ptr->stats.real_exclusive_locks_granted       = 0;
        xs_lock_ptr->stats.real_exclusive_locks_released      = 0;
        xs_lock_ptr->stats.max_exclusive_locks                = 0;
        xs_lock_ptr->stats.max_exclusive_lock_recursion_depth = 0;
        xs_lock_ptr->stats.exclusive_locks_delayed            = 0;
        xs_lock_ptr->stats.max_exclusive_locks_pending        = 0;
        xs_lock_ptr->stats.calls_to_x2s_func                  = 0;

        xs_lock_ptr->stats.shared_locks_granted               = 0;
        xs_lock_ptr->stats.shared_locks_coerced               = 0;
        xs_lock_ptr->stats.shared_locks_released              = 0;
        xs_lock_ptr->stats.real_shared_locks_granted          = 0;
        xs_lock_ptr->stats.real_shared_locks_released         = 0;
        xs_lock_ptr->stats.max_shared_locks                   = 0;
        xs_lock_ptr->stats.max_shared_lock_recursion_depth    = 0;
        xs_lock_ptr->stats.shared_locks_delayed               = 0;
        xs_lock_ptr->stats.max_shared_locks_pending           = 0;
    }

    return(ret_value);

} /* BPTS_pt_rec_xs_lock_init() */


/***************************************************************************
 * 
 * BPTS_pt_rec_xs_lock_takedown
 *
 *    Takedown an instance of BPTS_pt_rec_xs_lock_t.  All mutex, condition
 *    variables, and keys are destroyed, and tag is set to an invalid
 *    value.  However, neithr the x2s_data nor the instance of 
 *    BPTS_pt_rec_xs_lock_t is freed.
 *                                               JRM -- 4/23/25
 *
 *
 * RETURNS: 0 on success and non-zero on error.
 *
 **************************************************************************/

herr_t
BPTS_pt_rec_xs_lock_takedown(BPTS_pt_rec_xs_lock_t *xs_lock_ptr)
{
    herr_t ret_value = SUCCEED;

    if ( ( xs_lock_ptr == NULL ) ||
         ( xs_lock_ptr->tag != BPTS_PT_REC_XS_LOCK_TAG ) ||
         ( 0 != xs_lock_ptr->active_shared ) ||
         ( 0 != xs_lock_ptr->active_exclusive ) ||
         ( 0 != xs_lock_ptr->waiting_shared_count ) ||
         ( 0 != xs_lock_ptr->waiting_exclusive_count ) ) {

        ret_value = FAIL;

    } else {

        /* we are commited to the takedown at this point.  Set tag
         * to an invalid value, and call the appropriate pthread
         * destroy routines.  Call them all, even if one fails along
         * the way.
         */
        xs_lock_ptr->tag = 0;

        if ( ( pthread_mutex_destroy(&(xs_lock_ptr->mutex)) < 0 ) ||
             ( pthread_cond_destroy(&(xs_lock_ptr->shared_cv)) < 0 ) ||
             ( pthread_cond_destroy(&(xs_lock_ptr->exclusive_cv)) < 0 ) ||
             ( pthread_key_delete(xs_lock_ptr->rec_entry_count_key) < 0 ) ) {

            ret_value = FAIL;
        }
    }

    return(ret_value);

} /* BPTS_pt_rec_xs_lock_takedown() */


/***************************************************************************
 * 
 * BPTS_pt_rec_xs_shared_lock
 *
 *    Attempt to obtain a shared lock on the associated recursive exclusive
 *    shared lock.
 *
 *     The management of recursive locks in the Bypass VOL is a bit peculiar.
 *     While initial exclusive and shared lock requests must be granted as 
 *     such, recursive requests for either type of lock must be converted 
 *     to the type of lock already held by the requesting thread.
 *
 *     To see this, consider the following cases:
 *
 *     1) Suppose a thread that currently holds the exclusive lock executes 
 *        a user callback that invokes a HDF5 API that is usually executed 
 *        under a shared lock in the Bypass VOL.  Obviously, this API call 
 *        must be executed under the currently held exclusive lock to avoid 
 *        an immediate deadlock.
 *
 *     2) Now suppose that a thread with a shared lock executing in the 
 *        Bypass VOL has to make a call into the native VOL – as is currently 
 *        done to obtain the necessary metadata to perform I/O requests.  
 *        These calls have to be executed under the global mutex and thus 
 *        would normally be executed with the exclusive lock.  Assuming 
 *        such calls into the native VOL are routed through H5VL and then 
 *        back through the Bypass VOL, they would normally be executed 
 *        under the exclusive lock – which would cause an immediate deadlock.
 *
 *     Before granting an initial shared lock, we must test to see if 
 *     last_lock_exclusive is TRUE.  If it is, we must call the x2s_func
 *     if it is defined, and set last_lock_exclusive to FALSE before 
 *     granting the shared lock.
 *
 *                                                 JRM -- 4/23/25
 *
 * Returns: 0 on success and non-zero on error.
 *
 **************************************************************************/

herr_t
BPTS_pt_rec_xs_shared_lock(BPTS_pt_rec_xs_lock_t *xs_lock_ptr)
{
    bool have_mutex = FALSE;
    int result;
    BPTS_pt_rec_entry_count_t * count_ptr;
    herr_t ret_value = SUCCEED;

    if ( ( xs_lock_ptr == NULL ) ||
         ( xs_lock_ptr->tag != BPTS_PT_REC_XS_LOCK_TAG ) ) {

        ret_value = FAIL;
    }

    /* obtain the mutex */
    if ( ret_value == SUCCEED ) {

        if ( pthread_mutex_lock(&(xs_lock_ptr->mutex)) != 0 ) {

            ret_value = FAIL;

        } else {

            have_mutex = TRUE;
        }
    }

    /* If there is no specific data for this thread, this is an
     * initial read lock request.
     */
    if ( ret_value == SUCCEED ) {

        count_ptr = (BPTS_pt_rec_entry_count_t *)
                    pthread_getspecific(xs_lock_ptr->rec_entry_count_key);

        if ( count_ptr ) { /* this is a recursive lock */

            assert( count_ptr->tag == BPTS_PT_REC_XS_REC_ENTRY_COUNT_TAG );

            /* If the current lock is an exclusive lock, we must coerce the requested
             * shared lock into a shared lock, increment the recursive lock count,
             *  and update stats accordingly.
             *
             * In contrast, if the current lock is a shared lock, just increment 
             * the recursive lock count and update stats.
             */
            if ( count_ptr->exclusive_lock ) {

                assert( 0 == xs_lock_ptr->active_shared );
                assert( 1 == xs_lock_ptr->active_exclusive );

                count_ptr->rec_lock_count++;

                /* update stats */
                REC_XS_LOCK_STATS__UPDATE_FOR_EXCLUSIVE_LOCK(xs_lock_ptr, count_ptr, TRUE);

            } else {

                assert( 0 < xs_lock_ptr->active_shared );
                assert( 0 == xs_lock_ptr->active_exclusive );

                /* since this is a recursive shared lock, last_lock_exclusive must be FALSE */
                assert( ! xs_lock_ptr->last_lock_exclusive );

                count_ptr->rec_lock_count++;

                /* update stats */
                REC_XS_LOCK_STATS__UPDATE_FOR_SHARED_LOCK(xs_lock_ptr, count_ptr, FALSE);
            }
        } else { /* this is an initial shared lock request */

            switch ( xs_lock_ptr->policy ) {

                case BPTS__XS_LOCK_POLICY__FAVOR_EXCLUSIVE_ACCESS:
                    if ( ( xs_lock_ptr->active_exclusive != 0 ) ||
                         ( xs_lock_ptr->waiting_exclusive_count != 0 ) ) {

                        int delayed_cnt = xs_lock_ptr->waiting_shared_count + 1;

                        /* update stats */
                        REC_XS_LOCK_STATS__UPDATE_FOR_SHARED_LOCK_DELAY(xs_lock_ptr, delayed_cnt);
                    }

                    while ( ( xs_lock_ptr->active_exclusive != 0 ) ||
                            ( xs_lock_ptr->waiting_exclusive_count != 0 ) ) {

                        xs_lock_ptr->waiting_shared_count++;

                        result = pthread_cond_wait(&(xs_lock_ptr->shared_cv),
                                                   &(xs_lock_ptr->mutex));

                        xs_lock_ptr->waiting_shared_count--;

                        if ( result != 0 ) {

                            ret_value = FAIL;
                            break;
                        }
                    }
                    break;

                default:
                    ret_value = FAIL;
                    break;
            }

            if ( ( ret_value == SUCCEED ) &&
                 ( NULL == (count_ptr = BPTS_alloc_pt_rec_entry_count(FALSE)))){

                ret_value = FAIL;
            }

            if ( ( ret_value == SUCCEED ) &&
                 ( pthread_setspecific(xs_lock_ptr->rec_entry_count_key, (void *)count_ptr) != 0 ) ) {

                ret_value = FAIL;
            }

            if ( xs_lock_ptr->last_lock_exclusive ) {

                /* this must be the first shared lock granted since the previous exclusive lock */
                assert( 0 == xs_lock_ptr->active_shared );

                /* we are about to grant the first shared lock since the drop of the 
                 * the previous exclusive lock.
                 *
                 * In this circumstance, we must call the x2s_fcn() if it is defined.
                 */
                if ( xs_lock_ptr->x2s_func ) {

                    (xs_lock_ptr->x2s_func)(xs_lock_ptr->x2s_data);

                    /* update stats */
                    REC_XS_LOCK_STATS__UPDATE_FOR_CALL_TO_X2S_FUNC(xs_lock_ptr);
                }

                /* reset xs_lock_ptr->last_lock_exclusive to prevent unnecessary calls to x2s_func() */
                xs_lock_ptr->last_lock_exclusive = FALSE;
            }

            if ( ret_value == SUCCEED ) {

                xs_lock_ptr->active_shared++;

                assert(count_ptr->rec_lock_count == 1);

                /* update stats */
                REC_XS_LOCK_STATS__UPDATE_FOR_SHARED_LOCK(xs_lock_ptr, count_ptr, FALSE);
            }
        }
    }

    if ( have_mutex ) {

        pthread_mutex_unlock(&(xs_lock_ptr->mutex));
    }

    return(ret_value);

} /* BPTS_pt_rec_xs_shared_lock() */


/***************************************************************************
 * 
 * BPTS_pt_rec_xs_exclusive_lock
 *
 *    Attempt to obtain an exclusive lock on the associated recursive 
 *    exclusive shared lock.
 *
 *     The management of recursive locks in the Bypass VOL is a bit peculiar.
 *     While initial exclusive and shared lock requests must be granted as 
 *     such, recursive requests for either type of lock must be converted 
 *     to the type of lock already held by the requesting thread.
 *
 *     To see this, consider the following cases:
 *
 *     1) Suppose a thread that currently holds the exclusive lock executes 
 *        a user callback that invokes a HDF5 API that is usually executed 
 *        under a shared lock in the Bypass VOL.  Obviously, this API call 
 *        must be executed under the currently held exclusive lock to avoid 
 *        an immediate deadlock.
 *
 *     2) Now suppose that a thread with a shared lock executing in the 
 *        Bypass VOL has to make a call into the native VOL – as is currently 
 *        done to obtain the necessary metadata to perform I/O requests.  
 *        These calls have to be executed under the global mutex and thus 
 *        would normally be executed with the exclusive lock.  Assuming 
 *        such calls into the native VOL are routed through H5VL and then 
 *        back through the Bypass VOL, they would normally be executed 
 *        under the exclusive lock – which would cause an immediate deadlock.
 *
 *                                                 JRM -- 4/23/25
 *
 * Returns: 0 on success and non-zero on error.
 *
 **************************************************************************/

herr_t
BPTS_pt_rec_xs_exclusive_lock(BPTS_pt_rec_xs_lock_t *xs_lock_ptr)
{
    bool have_mutex = FALSE;
    int result;
    BPTS_pt_rec_entry_count_t * count_ptr;
    herr_t ret_value = SUCCEED;

    if ( ( xs_lock_ptr == NULL ) ||
         ( xs_lock_ptr->tag != BPTS_PT_REC_XS_LOCK_TAG ) ) {

        ret_value = FAIL;
    }

    /* obtain the mutex */
    if ( ret_value == SUCCEED ) {

        if ( pthread_mutex_lock(&(xs_lock_ptr->mutex)) != 0 ) {

            ret_value = FAIL;

        } else {

            have_mutex = TRUE;
        }
    }

    /* If there is no specific data for this thread, this is an
     * initial write lock request.
     */
    if ( ret_value == SUCCEED ) {

        count_ptr = (BPTS_pt_rec_entry_count_t *)
                    pthread_getspecific(xs_lock_ptr->rec_entry_count_key);

        if ( count_ptr ) { /* this is a recursive lock */

            assert((count_ptr)->tag == BPTS_PT_REC_XS_REC_ENTRY_COUNT_TAG);

            /* If the current lock is a shared lock, we must coerce the requested
             * exclusive lock into a shared lock, increment the recursive lock count,
             * and update stats accordingly.
             *
             * In contrast, if the current lock is an exclusive lock, just increment
             * the recursive lock count and update stats.
             */
            if ( count_ptr->exclusive_lock ) {

                assert( 0 == xs_lock_ptr->active_shared );
                assert( 1 == xs_lock_ptr->active_exclusive );

                count_ptr->rec_lock_count++;

                /* update stats */
                REC_XS_LOCK_STATS__UPDATE_FOR_EXCLUSIVE_LOCK(xs_lock_ptr, count_ptr, FALSE);

            } else {

                assert( 0 < xs_lock_ptr->active_shared );
                assert( 0 == xs_lock_ptr->active_exclusive );

                /* since this is a recursive shared lock, last_lock_exclusive must be FALSE */
                assert( ! xs_lock_ptr->last_lock_exclusive );

                count_ptr->rec_lock_count++;

                /* update stats */
                REC_XS_LOCK_STATS__UPDATE_FOR_SHARED_LOCK(xs_lock_ptr, count_ptr, TRUE);
            }
        } else { /* this is an initial write lock request */

            switch ( xs_lock_ptr->policy ) {

                case BPTS__XS_LOCK_POLICY__FAVOR_EXCLUSIVE_ACCESS:
                    if ( ( xs_lock_ptr->active_shared > 0 ) ||
                         ( xs_lock_ptr->active_exclusive > 0 ) ) {

                        int delayed = xs_lock_ptr->waiting_exclusive_count + 1;

                       REC_XS_LOCK_STATS__UPDATE_FOR_EXCLUSIVE_LOCK_DELAY(xs_lock_ptr, delayed);
                    }

                    while ( ( xs_lock_ptr->active_shared > 0 ) ||
                            ( xs_lock_ptr->active_exclusive > 0 ) ) {

                        xs_lock_ptr->waiting_exclusive_count++;

                        result = pthread_cond_wait(&(xs_lock_ptr->exclusive_cv),
                                                   &(xs_lock_ptr->mutex));

                        xs_lock_ptr->waiting_exclusive_count--;

                        if ( result != 0 ) {

                            ret_value = FAIL;
                            break;
                        }
                    }
                    break;

                default:
                    ret_value = FAIL;
                    break;
            }

            if ( ( ret_value == SUCCEED ) &&
                 ( NULL == (count_ptr = BPTS_alloc_pt_rec_entry_count(TRUE)))){

                ret_value = FAIL;
            }

            if ( ( ret_value == SUCCEED ) &&
                 ( pthread_setspecific(xs_lock_ptr->rec_entry_count_key, (void *)count_ptr) != 0 ) ) {

                ret_value = FAIL;
            }

            if ( ret_value == SUCCEED ) {

                xs_lock_ptr->active_exclusive++;

                assert(count_ptr->rec_lock_count == 1);

                REC_XS_LOCK_STATS__UPDATE_FOR_EXCLUSIVE_LOCK(xs_lock_ptr, count_ptr, FALSE);
            }
        }
    }

    if ( have_mutex ) {

        pthread_mutex_unlock(&(xs_lock_ptr->mutex));
    }

    return(ret_value);

} /* BPTS_pt_rec_xs_exclusive_lock() */

/***************************************************************************
 * 
 * BPTS_pt_rec_xs_unlock
 *
 *    Attempt to unlock either an exclusive or shared lock on the supplied
 *    recursive exclusive / shared lock.
 *
 *    If the current lock is an exclusive lock, and its recursive entry
 *    count drops to zero, causing it to be droped, set the 
 *    last_lock_exclusive flag before proceeding.
 *
 *    The last_lock_exclusive must be checked, and if necessary reset 
 *    before a shared lock is granted.  If it is set, and the x2s_func
 *    is defined, invoke the x2s_func() before proceeding.
 *
 *                                                JRM -- 4/24/25
 *
 * Returns: 0 on success and non-zero on error.
 *
 **************************************************************************/

herr_t
BPTS_pt_rec_xs_unlock(BPTS_pt_rec_xs_lock_t *xs_lock_ptr)
{
    bool have_mutex = FALSE;
    bool discard_rec_count = FALSE;
    BPTS_pt_rec_entry_count_t * count_ptr;
    herr_t ret_value = SUCCEED;

    if ( ( xs_lock_ptr == NULL ) ||
         ( xs_lock_ptr->tag != BPTS_PT_REC_XS_LOCK_TAG ) ) {

        ret_value = FAIL;
    }

    /* obtain the mutex */
    if ( ret_value == SUCCEED ) {

        if ( pthread_mutex_lock(&(xs_lock_ptr->mutex)) != 0 ) {

            ret_value = FAIL;

        } else {

            have_mutex = TRUE;
        }
    }

    /* If there is no specific data for this thread, no lock was held,
     * and thus the unlock call must fail.
     */
    if ( ret_value == SUCCEED ) {

        count_ptr = (BPTS_pt_rec_entry_count_t *)
                    pthread_getspecific(xs_lock_ptr->rec_entry_count_key);

        assert( count_ptr );
        assert( count_ptr->tag == BPTS_PT_REC_XS_REC_ENTRY_COUNT_TAG );
        assert( count_ptr->rec_lock_count > 0 );

        if ( NULL == count_ptr ) {

             ret_value = FAIL;

        } else if ( count_ptr->tag != BPTS_PT_REC_XS_REC_ENTRY_COUNT_TAG ) {

            ret_value = FAIL;

        } else if ( count_ptr->rec_lock_count <= 0 ) { /* corrupt count? */

            ret_value = FAIL;

        } else if ( count_ptr->exclusive_lock ) { /* drop an exclusive lock */

            assert( xs_lock_ptr->active_shared == 0 );
            assert( xs_lock_ptr->active_exclusive == 1 );

            if ( ( xs_lock_ptr->active_shared != 0 ) ||
                 ( xs_lock_ptr->active_exclusive != 1 ) ) {

                ret_value = FAIL;

            } else {

                count_ptr->rec_lock_count--;

                assert(count_ptr->rec_lock_count >= 0);

                if ( count_ptr->rec_lock_count == 0 ) {

                    /* make note that we must discard the
                     * recursive entry counter so it will not
                     * confuse us on the next lock request.
                     */
                    discard_rec_count = TRUE;

                    /* drop the write lock -- will signal later if needed */
                    xs_lock_ptr->active_exclusive--;

                    /* set the last_lock_exclusive flag, so as to trigger a 
                     * a call to the x2s_func() if the next lock granted is
                     * a shared lock.
                     */
                    xs_lock_ptr->last_lock_exclusive = TRUE;

                    assert(xs_lock_ptr->active_exclusive == 0);
                }
            }

            REC_XS_LOCK_STATS__UPDATE_FOR_EXCLUSIVE_UNLOCK(xs_lock_ptr, count_ptr);

        } else { /* drop a read lock */

            assert( ( xs_lock_ptr->active_shared > 0 ) &&
                    ( xs_lock_ptr->active_exclusive == 0 ) );

            if ( ( xs_lock_ptr->active_shared <= 0 ) ||
                 ( xs_lock_ptr->active_exclusive != 0 ) ) {

                ret_value = FAIL;

            } else {

                count_ptr->rec_lock_count--;

                assert(count_ptr->rec_lock_count >= 0);

                if ( count_ptr->rec_lock_count == 0 ) {

                    /* make note that we must discard the
                     * recursive entry counter so it will not
                     * confuse us on the next lock request.
                     */
                    discard_rec_count = TRUE;

                    /* drop the read lock -- will signal later if needed */
                    xs_lock_ptr->active_shared--;
                }
            }

            REC_XS_LOCK_STATS__UPDATE_FOR_SHARED_UNLOCK(xs_lock_ptr, count_ptr);
        }

        if ( ( ret_value == SUCCEED ) &&
             ( xs_lock_ptr->active_shared == 0 ) &&
             ( xs_lock_ptr->active_exclusive == 0 ) ) {

            /* no locks held -- signal condition variables if required */

            switch ( xs_lock_ptr->policy ) {

                case BPTS__XS_LOCK_POLICY__FAVOR_EXCLUSIVE_ACCESS:

                    if ( xs_lock_ptr->waiting_exclusive_count > 0 ) {

                        if ( pthread_cond_signal(&(xs_lock_ptr->exclusive_cv)) != 0 ) {

                            ret_value = FAIL;
                        }
                    } else {

                        /* signal any threads waiting on the shared condition variable */
                        if ( xs_lock_ptr->waiting_shared_count > 0 ) {

                            if ( pthread_cond_broadcast(&(xs_lock_ptr->shared_cv)) != 0 ) {

                                ret_value = FAIL;
                            }
                        }
                    }
                    break;

                default:
                    ret_value = FAIL;
                    break;
            }
        }
    }

    /* if we are really dropping the lock, must set the value of
     * rec_entry_count_key for this thread to NULL, so that
     * when this thread next requests a lock, it will appear
     * as an initial lock, not a recursive lock.
     */
    if ( discard_rec_count ) {

        assert(count_ptr);

        if ( pthread_setspecific(xs_lock_ptr->rec_entry_count_key, (void *)NULL) != 0 ) {

            ret_value = FAIL;
        }

        BPTS_free_pt_rec_entry_count((void *)count_ptr);
        count_ptr = NULL;
    }

    if ( have_mutex ) {

        pthread_mutex_unlock(&(xs_lock_ptr->mutex));
    }

    return(ret_value);

} /* BPTS_pt_rec_xs_unlock() */


/***************************************************************************
 *
 * BPTS_pt_rec_xs_lock_get_stats
 * 
 *    Obtain a copy of the current statistics on the supplied
 *    recursive exclusive / shared lock.  Note that to obtain a consistent
 *    set of statistics, the function must obtain the lock mutex.
 *
 *                                          JRM -- 4/25/25
 *
 * RETURNS: 0 on success and non-zero on error.
 *
 **************************************************************************/

herr_t
BPTS_pt_rec_xs_lock_get_stats(BPTS_pt_rec_xs_lock_t *xs_lock_ptr,
    BPTS_pt_rec_xs_lock_stats_t * stats_ptr)
{
    bool have_mutex = FALSE;
    herr_t ret_value = SUCCEED;

    if ( ( xs_lock_ptr == NULL ) ||
         ( xs_lock_ptr->tag != BPTS_PT_REC_XS_LOCK_TAG ) ||
         ( stats_ptr == NULL ) ) {

        ret_value = FAIL;
    }

    /* obtain the mutex */
    if ( ret_value == SUCCEED ) {

        if ( pthread_mutex_lock(&(xs_lock_ptr->mutex)) != 0 ) {

            ret_value = FAIL;

        } else {

            have_mutex = TRUE;
        }
    }

    if ( ret_value == SUCCEED ) {

        *stats_ptr = xs_lock_ptr->stats;
    }

    if ( have_mutex ) {

        pthread_mutex_unlock(&(xs_lock_ptr->mutex));
    }

    return(ret_value);

} /* BPTS_pt_rec_xs_lock_get_stats() */


/***************************************************************************
 * 
 * BPTS_pt_rec_rw_lock_reset_stats
 *
 *    Reset the statistics for the supplied exclusive / shared lock.
 *    Note that to reset the statistics consistently, the function must
 *    obtain the lock mutex.
 *                                            JRM -- 4/25/25
 *
 * RETURNS: 0 on success and non-zero on error.
 *
 **************************************************************************/

herr_t
BPTS_pt_rec_xs_lock_reset_stats(BPTS_pt_rec_xs_lock_t *xs_lock_ptr)
{
    bool have_mutex = FALSE;
    /* update this initializer if you modify BPTS_pt_rec_xs_lock_stats_t */
    static const BPTS_pt_rec_xs_lock_stats_t reset_stats = {
        /* exclusive_locks_granted            = */ 0,
        /* exclusive_locks_coerced            = */ 0,
        /* exclusive_locks_released           = */ 0,
        /* real_exclusive_locks_granted       = */ 0,
        /* real_exclusive_locks_released      = */ 0,
        /* max_exclusive_locks                = */ 0,
        /* max_exclusive_lock_recursion_depth = */ 0,
        /* exclusive_locks_delayed            = */ 0,
        /* max_exclusive_locks_pending        = */ 0,
        /* calls_to_x2s_func                  = */ 0,
        /* shared_locks_granted               = */ 0,
        /* shared_locks_coerced               = */ 0,
        /* shared_locks_released              = */ 0,
        /* real_shared_locks_granted          = */ 0,
        /* real_shared_locks_released         = */ 0,
        /* max_shared_locks                   = */ 0,
        /* max_shared_lock_recursion_depth    = */ 0,
        /* shared_locks_delayed               = */ 0,
        /* max_shared_locks_pending           = */ 0
    };
    herr_t ret_value = SUCCEED;

    if ( ( xs_lock_ptr == NULL ) ||
         ( xs_lock_ptr->tag != BPTS_PT_REC_XS_LOCK_TAG ) ) {

        ret_value = FAIL;
    }

    /* obtain the mutex */
    if ( ret_value == SUCCEED ) {

        if ( pthread_mutex_lock(&(xs_lock_ptr->mutex)) != 0 ) {

            ret_value = FAIL;

        } else {

            have_mutex = TRUE;
        }
    }

    if ( ret_value == SUCCEED ) {

        xs_lock_ptr->stats = reset_stats;
    }

    if ( have_mutex ) {

        pthread_mutex_unlock(&(xs_lock_ptr->mutex));
    }

    return(ret_value);

} /* BPTS_pt_rec_xs_lock_reset_stats() */


/***************************************************************************
 *
 * BPTS_pt_rec_rw_lock_print_stats
 *
 *    Print the supplied pthresds recursive X/S lock statistics to
 *    standard out.
 *
 *    UPDATE THIS FUNCTION IF YOU MODIFY BPTS_pt_rec_xs_lock_stats_t.
 *
 *                                                 JRM -- 4/25/25
 *
 * RETURNS: 0 on success and non-zero on error.
 *
 **************************************************************************/

herr_t
BPTS_pt_rec_xs_lock_print_stats(const char * header_str,
    BPTS_pt_rec_xs_lock_stats_t * stats_ptr)
{
    herr_t ret_value = SUCCEED;

    if ( ( header_str == NULL ) ||
         ( stats_ptr == NULL ) ) {

        ret_value = FAIL;

    } else {

        fprintf(stdout, "\n\n%s\n\n", header_str);

        fprintf(stdout, "  exclusive_locks_granted            = %lld\n",
                (long long int)(stats_ptr->exclusive_locks_granted));
        fprintf(stdout, "  exclusive_locks_coerced            = %lld\n",
                (long long int)(stats_ptr->exclusive_locks_coerced));
        fprintf(stdout, "  exclusive_locks_released           = %lld\n",
                (long long int)(stats_ptr->exclusive_locks_released));
        fprintf(stdout, "  real_exclusive_locks_granted       = %lld\n",
                (long long int)(stats_ptr->real_exclusive_locks_granted));
        fprintf(stdout, "  real_exclusive_locks_released      = %lld\n",
                (long long int)(stats_ptr->real_exclusive_locks_released));
        fprintf(stdout, "  max_exclusive_locks                = %lld\n",
                (long long int)(stats_ptr->max_exclusive_locks));
        fprintf(stdout, "  max_exclusive_lock_recursion_depth = %lld\n",
                stats_ptr->max_exclusive_lock_recursion_depth);
        fprintf(stdout, "  exclusive_locks_delayed            = %lld\n",
                stats_ptr->exclusive_locks_delayed);
        fprintf(stdout, "  max_exclusive_locks_pending        = %lld\n",
                stats_ptr->max_exclusive_locks_pending);
        fprintf(stdout, "  calls_to_x2s_func                  = %lld\n\n",
                stats_ptr->calls_to_x2s_func);

        fprintf(stdout, "  shared_locks_granted               = %lld\n",
                (long long int)(stats_ptr->shared_locks_granted));
        fprintf(stdout, "  shared_locks_coerced               = %lld\n",
                (long long int)(stats_ptr->shared_locks_coerced));
        fprintf(stdout, "  shared_locks_released              = %lld\n",
                (long long int)(stats_ptr->shared_locks_released));
        fprintf(stdout, "  real_shared_locks_granted          = %lld\n",
                (long long int)(stats_ptr->real_shared_locks_granted));
        fprintf(stdout, "  real_shared_locks_released         = %lld\n",
                (long long int)(stats_ptr->real_shared_locks_released));
        fprintf(stdout, "  max_shared_locks                   = %lld\n",
                (long long int)(stats_ptr->max_shared_locks));
        fprintf(stdout, "  max_shared_lock_recursion_depth    = %lld\n",
                (long long int)(stats_ptr->max_shared_lock_recursion_depth));
        fprintf(stdout, "  shared_locks_delayed               = %lld\n",
                (long long int)(stats_ptr->shared_locks_delayed));
        fprintf(stdout, "  max_shared_locks_pending           = %lld\n\n",
                (long long int)(stats_ptr->max_shared_locks_pending));
    }

    return(ret_value);

} /* BPTS_pt_rec_xs_lock_print_stats() */


