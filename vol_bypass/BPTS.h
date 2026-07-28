#if 1 /* delete once integrated */

typedef int herr_t;
typedef unsigned int uint32_t;
#define SUCCEED    0
#define FAIL    (-1)
#define FALSE false
#define TRUE true

#endif /* delete once integrated */

/******************************************************************************
 *
 * p-thread recursive X/S lock stats collection macros
 *
 * Macros to maintain statistics on the p-threads recursive exclusive / 
 * shared lock
 *
 ******************************************************************************/

#define REC_XS_LOCK_STATS__UPDATE_FOR_SHARED_LOCK(xs, count_ptr, coerced)     \
do {                                                                          \
    assert(xs);                                                               \
    assert((xs)->tag == BPTS_PT_REC_XS_LOCK_TAG);                             \
    assert(count_ptr);                                                        \
    assert((count_ptr)->tag == BPTS_PT_REC_XS_REC_ENTRY_COUNT_TAG);           \
    assert((count_ptr)->rec_lock_count >= 1);                                 \
    assert(!(count_ptr)->exclusive_lock);                                     \
                                                                              \
    (xs)->stats.shared_locks_granted++;                                       \
                                                                              \
    if ( coerced ) {                                                          \
                                                                              \
        assert( (count_ptr)->rec_lock_count > 1 );                            \
        (xs)->stats.shared_locks_coerced++;                                   \
    }                                                                         \
                                                                              \
    if ( (count_ptr)->rec_lock_count == 1) {                                  \
                                                                              \
        (xs)->stats.real_shared_locks_granted++;                              \
                                                                              \
        if ( (xs)->active_shared > (xs)->stats.max_shared_locks ) {           \
                                                                              \
            (xs)->stats.max_shared_locks = (xs)->active_shared;               \
        }                                                                     \
    }                                                                         \
                                                                              \
    if ( (count_ptr)->rec_lock_count >                                        \
         (xs)->stats.max_shared_lock_recursion_depth ) {                      \
                                                                              \
        (xs)->stats.max_shared_lock_recursion_depth =                         \
            (count_ptr)->rec_lock_count;                                      \
    }                                                                         \
} while ( FALSE ) /* end REC_XS_LOCK_STATS__UPDATE_FOR_SHARED_LOCK */


#define REC_XS_LOCK_STATS__UPDATE_FOR_SHARED_LOCK_DELAY(xs, waiting_count)    \
do {                                                                          \
    assert(xs);                                                               \
    assert((xs)->tag == BPTS_PT_REC_XS_LOCK_TAG);                             \
    assert((waiting_count) > 0);                                              \
                                                                              \
    (xs)->stats.shared_locks_delayed++;                                       \
                                                                              \
    if ( (xs)->stats.max_shared_locks_pending < (waiting_count) ) {           \
                                                                              \
        (xs)->stats.max_shared_locks_pending = (waiting_count);               \
    }                                                                         \
} while ( FALSE ) /* REC_XS_LOCK_STATS__UPDATE_FOR_SHARED_LOCK_DELAY */


#define REC_XS_LOCK_STATS__UPDATE_FOR_SHARED_UNLOCK(xs, count_ptr)            \
do {                                                                          \
    assert(xs);                                                               \
    assert((xs)->tag == BPTS_PT_REC_XS_LOCK_TAG);                             \
    assert(count_ptr);                                                        \
    assert((count_ptr)->tag == BPTS_PT_REC_XS_REC_ENTRY_COUNT_TAG);           \
    assert((count_ptr)->rec_lock_count >= 0);                                 \
    assert(!(count_ptr)->exclusive_lock);                                     \
                                                                              \
    (xs)->stats.shared_locks_released++;                                      \
                                                                              \
    if ( count_ptr->rec_lock_count == 0) {                                    \
                                                                              \
        (xs)->stats.real_shared_locks_released++;                             \
    }                                                                         \
} while ( FALSE ) /* end REC_XS_LOCK_STATS__UPDATE_FOR_SHARED_UNLOCK */


#define REC_XS_LOCK_STATS__UPDATE_FOR_EXCLUSIVE_LOCK(xs, count_ptr, coerced)  \
do {                                                                          \
    assert(xs);                                                               \
    assert((xs)->tag == BPTS_PT_REC_XS_LOCK_TAG);                             \
    assert(count_ptr);                                                        \
    assert((count_ptr)->tag == BPTS_PT_REC_XS_REC_ENTRY_COUNT_TAG);           \
    assert((count_ptr)->rec_lock_count >= 1);                                 \
    assert((count_ptr)->exclusive_lock);                                      \
                                                                              \
    (xs)->stats.exclusive_locks_granted++;                                    \
                                                                              \
    if ( coerced ) {                                                          \
                                                                              \
        assert( (count_ptr)->rec_lock_count > 1 );                            \
        (xs)->stats.exclusive_locks_coerced++;                                \
    }                                                                         \
                                                                              \
    if ( (count_ptr)->rec_lock_count == 1) {                                  \
                                                                              \
        (xs)->stats.real_exclusive_locks_granted++;                           \
                                                                              \
        if ( (xs)->active_exclusive > (xs)->stats.max_exclusive_locks ) {     \
                                                                              \
            (xs)->stats.max_exclusive_locks = (xs)->active_exclusive;         \
        }                                                                     \
    }                                                                         \
                                                                              \
    if ( (count_ptr)->rec_lock_count >                                        \
         (xs)->stats.max_exclusive_lock_recursion_depth ) {                   \
                                                                              \
        (xs)->stats.max_exclusive_lock_recursion_depth =                      \
            (count_ptr)->rec_lock_count;                                      \
    }                                                                         \
} while ( FALSE ) /* end REC_XS_LOCK_STATS__UPDATE_FOR_EXCLUSIVE_LOCK */


#define REC_XS_LOCK_STATS__UPDATE_FOR_EXCLUSIVE_LOCK_DELAY(xs, waiting_count) \
do {                                                                          \
    assert(xs);                                                               \
    assert((xs)->tag == BPTS_PT_REC_XS_LOCK_TAG);                             \
    assert((waiting_count) > 0);                                              \
                                                                              \
    (xs)->stats.exclusive_locks_delayed++;                                    \
                                                                              \
    if ( (xs)->stats.max_exclusive_locks_pending < (waiting_count) ) {        \
                                                                              \
        (xs)->stats.max_exclusive_locks_pending = (waiting_count);            \
    }                                                                         \
} while ( FALSE ) /* REC_XS_LOCK_STATS__UPDATE_FOR_EXCLUSIVE_LOCK_DELAY */


#define REC_XS_LOCK_STATS__UPDATE_FOR_EXCLUSIVE_UNLOCK(xs, count_ptr)         \
do {                                                                          \
    assert(xs);                                                               \
    assert((xs)->tag == BPTS_PT_REC_XS_LOCK_TAG);                             \
    assert(count_ptr);                                                        \
    assert((count_ptr)->tag == BPTS_PT_REC_XS_REC_ENTRY_COUNT_TAG);           \
    assert((count_ptr)->rec_lock_count >= 0);                                 \
    assert((count_ptr)->exclusive_lock);                                      \
                                                                              \
    (xs)->stats.exclusive_locks_released++;                                   \
                                                                              \
    if ( (count_ptr)->rec_lock_count == 0) {                                  \
                                                                              \
        (xs)->stats.real_exclusive_locks_released++;                          \
    }                                                                         \
} while ( FALSE ) /* end REC_XS_LOCK_STATS__UPDATE_FOR_EXCLUSIVE_UNLOCK */


#define REC_XS_LOCK_STATS__UPDATE_FOR_CALL_TO_X2S_FUNC(xs)                    \
do {                                                                          \
    assert(xs);                                                               \
    assert((xs)->tag == BPTS_PT_REC_XS_LOCK_TAG);                             \
    assert((xs)->last_lock_exclusive);                                        \
                                                                              \
    (xs)->stats.calls_to_x2s_func++;                                          \
                                                                              \
} while ( FALSE ) /* end REC_XS_LOCK_STATS__UPDATE_FOR_CALL_TO_X2S_FUNC */


/******************************************************************************
 *
 * Structure BPTS_pt_rec_xs_lock_stats_t
 *
 * Catchall structure for statistics on the recursive p-threads based
 * recursive exclusive / shared lock (see declaration of BPTS_pt_rec_xs_lock_t 
 * below).
 *
 * Since the mutex must be held when reading a consistent set of statistics
 * from the recursibe X/S lock, it simplifies matters to bundle them into
 * a single structure.  This structure exists for that purpose.
 *
 * If you modify this structure, be sure to make equivalent changes to
 * the reset_stats initializer in BPTS_pt_rec_xs_lock_reset_stats().
 *
 * Individual fields are discussed below.
 *
 *                                           JRM -- 4/23/25
 *
 *
 * exclusive lock stats:
 *
 * exclusive_locks_granted: 64 bit integer used to count the total number of 
 *              exclusive locks granted.  Note that this includes recursive 
 *              lock requests.
 *
 * exclusive_locks_coerced: 64 bit integer used to count the total number of 
 *              recursive shared lock requests granted as exclusive locks.
 *
 * exclusive_locks_released: 64 bit integer used to count the total number of 
 *              exclusive locks released.  Note that this includes recursive 
 *              lock release requests.
 *
 * real_exclusive_locks_granted: 64 bit integer used to count the total number
 *              of exlusive locks granted, less any recursive lock requests.
 *
 * real_exclusive_locks_released:  64 bit integer used to count the total number of
 *              exclusive locks released, less any recursive lock releases.
 *
 * max_exclusive_locks; 64 bit integer used to track the maximum number of 
 *              exclusive locks active at any point in time.  Must be either 
 *              zero or one.
 *
 * max_exclusive_lock_recursion_depth; 64 bit integer used to track the maximum
 *              recursion depth observed for any exclusive lock.
 *
 * exclusive_locks_delayed: 64 bit integer used to track the number of exclusive
 *              locks that were not granted immediately.
 *
 * max_exclusive_locks_delayed; 64 bit integer used to track the maximum number 
 *              of pending exclusive locks at any point in time.
 *
 * calls_to_x2s_func: 64 bit integer used to track the number of calls to the 
 *              x2s_func.  If defined, this function must be called whenever
 *              an exclusive lock is dropped, and either replaced with a shared 
 *              lock, or not replaced immediately.
 *
 *
 * Shared lock stats:
 *
 * shared_locks_granted: 64 bit integer used to count the total number of 
 *              shared locks granted.  Note that this includes recursive 
 *              lock requests.
 *
 * shared_locks_coerced: 64 bit integer used to count the total number of 
 *              recursive exclusive lock requests granted as shared locks.
 *
 * shared_locks_released: 64 bit integer used to count the total number of 
 *              shared locks released.  Note that this includes recursive 
 *              lock release requests.
 *
 * real_shared_locks_granted: 64 bit integer used to count the total number 
 *              of shared locks granted, less any recursive lock requests.
 *
 * real_shared_locks_released:  64 bit integer used to count the total number 
 *              of shared locks released, less any recursive lock releases.
 *
 * max_shared_locks; 64 bit integer used to track the maximum number of 
 *              shared locks active at any point in time.
 *
 * max_shared_lock_recursion_depth; 64 bit integer used to track the maximum
 *              recursion depth observed for any shared lock.
 *
 * shared_locks_delayed: 64 bit integer used to track the number of shared 
 *              locks that were not granted immediately.
 *
 * max_shared_locks_delayed; 64 bit integer used to track the maximum number 
 *              of pending shared locks at any point in time.
 *
 ******************************************************************************/

typedef struct BPTS_pt_rec_xs_lock_stats_t {

    int64_t             exclusive_locks_granted;
    int64_t             exclusive_locks_coerced;
    int64_t             exclusive_locks_released;
    int64_t             real_exclusive_locks_granted;
    int64_t             real_exclusive_locks_released;
    int64_t             max_exclusive_locks;
    int64_t             max_exclusive_lock_recursion_depth;
    int64_t             exclusive_locks_delayed;
    int64_t             max_exclusive_locks_pending;
    int64_t             calls_to_x2s_func;

    int64_t             shared_locks_granted;
    int64_t             shared_locks_coerced;
    int64_t             shared_locks_released;
    int64_t             real_shared_locks_granted;
    int64_t             real_shared_locks_released;
    int64_t             max_shared_locks;
    int64_t             max_shared_lock_recursion_depth;
    int64_t             shared_locks_delayed;
    int64_t             max_shared_locks_pending;

} BPTS_pt_rec_xs_lock_stats_t;

/******************************************************************************
 *
 * Structure BPTS_pt_rec_xs_lock_t
 *
 * An exclusive / shared lock is a lock that allows either an arbitrary number 
 * of threads shared access to the protected critical region, or a single 
 * thread exclusive access.  Note that such locks are usually referred to 
 * as read / write locks.  However, given the application of the exclusive / 
 * shared lock in the Bypass VOL, using the typical name would be confusing.
 *
 * A recursive lock is one that allows a thread that already has a lock (be it
 * exclusive or shared) to successfully request the lock again, only dropping 
 * the lock when the number of un-lock calls equals the number of lock calls.
 *
 * The management of recursive locks in the Bypass VOL is a bit peculiar.  
 * While initial exclusive and shared lock request must be granted as such, 
 * recursive requests for either type of lock must be converted to the type of 
 * lock already held by the requesting thread. 
 *
 * To see this, consider the following cases:
 *
 * 1) Suppose a thread that currently holds the exclusive lock executes a 
 *    a user callback that invokes a HDF5 API that is usually executed under
 *    a shared lock in the Bypass VOL.  Obviously, this API call must be 
 *    executed under the currently held exclusive lock to avoid an immediate
 *    deadlock.
 *
 * 2) Now suppose that a thread with a shared lock executing in the Bypass VOL 
 *    has to make a call into the native VOL – as is currently done to obtain 
 *    the necessary metadata to perform I/O requests.  These calls have to 
 *    be executed under the global mutex and thus would normally be executed 
 *    with the exclusive lock.  Assuming such calls into the native VOL are 
 *    routed through H5VL and then back through the Bypass VOL, they would 
 *    normally be executed under the exclusive lock – which would cause an 
 *    immediate deadlock. 
 *
 * Finally, note that we can't use the p-threads R/W lock as the base of the 
 * exclusive / shared lock, as while it permits recursive read locks, it 
 * disallows recursive write locks.
 *
 * This structure is a catchall for the fields needed to implement a 
 * p-threads based recursive exclusive / shared lock, and for the 
 * associated statistics collection fields.
 *
 * This recursive exclusive / shared lock implementation is an extension of 
 * the R/W lock implementation given in "UNIX network programming" Volume 2, 
 * Chapter 8 by w. Richard Stevens, 2nd edition.
 *
 * Individual fields are discussed below.
 *
 *                                           JRM  -- 4/17/25
 *
 * tag:         Unsigned 32 bit integer field used for sanity checking.  This
 *              field must always be set to ???_PT_REC_XS_LOCK_TAG.  
 *              If this structure is allocated dynamically, remember to set 
 *              it to some invalid value before discarding the structure.
 * 
 * policy       Integer containing a code indicating the precedence policy
 *              used by the exclusive / shared lock.  The supported policies 
 *              are listed below:
 *
 *              BPTS__XS_LOCK_POLICY__FAVOR_EXCLUSIVE_ACCESS:
 *
 *              If selected, the exclusive / shared lock will grant a
 *              pending exclusive lock request if there are both pending 
 *              shared and exclusive lock requests.
 *              
 *              
 *              --- Define other policies here ---
 *              
 *
 * mutex:       Mutex used to maintain mutual exclusion on the fields of 
 *              of this structure.
 *
 * shared_cv:  Condition variable used for threads waiting for shared access.
 *
 * exclusive_cv: Condition variable used for threads waiting for exclusive 
 *		    access.
 * 
 * waiting_shared_count: 32 bit integer used to maintain a count of the number
 *              of threads waiting for shared access.  This value should always 
 *              be non-negative.
 *
 * waiting_exclusive_count: 32 bit integer used to maintain a count of the 
 *              number of threads waiting for exclusive access.  This value 
 *              should always be non-negative.
 *
 * The following two fields could be combined into a single field, with
 * the count of threads with shared access being represented by a positive 
 * value, and the number of threads with exclusive access by a negative value.
 * Two fields are used to facilitate sanity checking.
 * 
 * active_shared: 32 bit integer used to maintain a count of the number of 
 *              threads that currently hold a shared lock.  This value 
 *              must be zero if active_exclusive is positive. It should
 *              never be negative.
 *
 * active_exclusive: 32 bit integer used to maintain a count of the number of
 *              threads that currently hold an exclusive lock.  This value 
 *              must always be either 0 or 1, and must be zero if 
 *              active_shared is positive.  It should never be negative.
 *              
 * rec_entry_count_key: Instance of pthread_key_t used to maintain
 *              a thread specific lock type and recursive entry count 
 *              for all threads holding a lock.
 *
 * last_lock_exclusive:  Boolean flag that is set to TRUE whenever an exclusve
 *              lock is dropped, and set to false whenever a shared lock is 
 *              granted.
 *
 *              If a shared lock is about to be granted, the last_lock_exclusive
 *              field is TRUE, and the x2s_func field is not NULL, the x2s_func
 *              must be called immediately before the last_lock_exclusive is set
 *              to false, and the shared lock is granted.
 *
 *              Note that last_lock_exclusive must always be false if 
 *              active_shared is positive
 *
 * x2s_func:    Pointer to a function that must be called when a thread with 
 *              an exclusive lock drops it, and one or more other threads 
 *              obtain the shared lock.  Note that this function must complete
 *              before any thread is given the shared lock.
 *
 * x2s_data:    Void pointer that is passed to x2s_func when it is invoked.
 *
 * stats:       Instance of BPTS_pt_rec_xs_lock_stats_t used to track 
 *              statistics on the recursive exclusive / shared lock.  See 
 *              the declaration of the structure for discussion of its fields.
 *
 *              Note that the stats are gathered into a structure because
 *              we must obtain the mutex when reading the statistics to 
 *              avoid changes while the statistics are being read.  Collecting
 *              them into a structure facilitates this.
 *
 ******************************************************************************/

#define BPTS_PT_REC_XS_LOCK_TAG                      0XABCD

#define BPTS__XS_LOCK_POLICY__FAVOR_EXCLUSIVE_ACCESS 0

typedef herr_t (BPTS_pt_rec_xs_x2s_func_t)(void * data); 

typedef struct BPTS_pt_rec_xs_lock_t {

    uint32_t                            tag;
    int32_t                             policy;
    pthread_mutex_t                     mutex;
    pthread_cond_t                      shared_cv;
    pthread_cond_t                      exclusive_cv;
    int32_t                             waiting_shared_count;
    int32_t                             waiting_exclusive_count;
    int32_t                             active_shared;
    int32_t                             active_exclusive;
    pthread_key_t                       rec_entry_count_key;
    int32_t                             exclusive_rec_entry_count;
    bool                                last_lock_exclusive;
    BPTS_pt_rec_xs_x2s_func_t *         x2s_func;
    void *                              x2s_data;
    struct BPTS_pt_rec_xs_lock_stats_t  stats;

} BPTS_pt_rec_xs_lock_t;


/******************************************************************************
 *
 * Structure BPTS_pt_rec_entry_count_t
 *
 * Structure associated with the rec_entry_count_key defined in 
 * BPTS_pt_rec_xs_lock_t.  
 *
 * The primary purpose of this structure is to maintain a count of recursive
 * locks so that the lock can be dropped when the count drops to zero.
 *
 * Additional fields are included for purposes of sanity checking.
 *
 * Individual fields are discussed below.
 *
 *                                           JRM -- 4/17/25
 *
 * tag:         Unsigned 32 bit integer field used for sanity checking.  This
 *              fields must always be set to 
 *              PBTS_PT_REC_XS_REC_ENTRY_COUNT_TAG, and should be set to 
 *              some invalid value just before the structure is freed.
 *
 * exclusive_lock:  Boolean field that is set to TRUE if the count is for an
 *              exclusive lock, and to FALSE if it is for a shared lock.
 *
 * rec_lock_count: Count of the number of recursive lock calls, less 
 *              the number of recursive unlock calls.  The lock in question
 *              is dropped when the count drops to zero.
 *
 ******************************************************************************/

#define BPTS_PT_REC_XS_REC_ENTRY_COUNT_TAG       0XABBA

typedef struct PBTS_pt_rec_entry_count_t {

    uint32_t    tag;
    bool        exclusive_lock;
    int64_t     rec_lock_count;

} BPTS_pt_rec_entry_count_t;

herr_t BPTS_pt_rec_xs_lock_init(BPTS_pt_rec_xs_lock_t *xs_lock_ptr, int policy,
                         BPTS_pt_rec_xs_x2s_func_t x2s_func, void * x2s_data);

herr_t BPTS_pt_rec_xs_lock_takedown(BPTS_pt_rec_xs_lock_t *xs_lock_ptr);

herr_t BPTS_pt_rec_xs_shared_lock(BPTS_pt_rec_xs_lock_t *xs_lock_ptr);

herr_t BPTS_pt_rec_xs_exclusive_lock(BPTS_pt_rec_xs_lock_t *xs_lock_ptr);

herr_t BPTS_pt_rec_xs_unlock(BPTS_pt_rec_xs_lock_t *xs_lock_ptr);

herr_t BPTS_pt_rec_xs_lock_get_stats(BPTS_pt_rec_xs_lock_t *xs_lock_ptr,
        BPTS_pt_rec_xs_lock_stats_t * stats_ptr);

herr_t BPTS_pt_rec_xs_lock_print_stats(const char * header_str,
        BPTS_pt_rec_xs_lock_stats_t * stats_ptr);

herr_t BPTS_pt_rec_xs_lock_reset_stats(BPTS_pt_rec_xs_lock_t *xs_lock_ptr);

