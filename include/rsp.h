/**
 * @file rsp.h
 * @brief Hardware Coprocessor Interface
 * @ingroup rsp
 */
#ifndef __LIBDRAGON_RSP_H
#define __LIBDRAGON_RSP_H

/**
 * @addtogroup rsp
 * @{
 */

/**
 * @brief Internal Job structure
 */
typedef struct ijob {
    struct ijob *prev;
    struct ijob *next;

    void (*cb)(struct ijob *);

    volatile uint32_t state;
    uint32_t fn;
    uint32_t args[16];

} job_t;

/**
 * @brief Job states
 */
#define JOB_STATE_IDLE     0
#define JOB_STATE_QUEUED   1
#define JOB_STATE_RUNNING  2
#define JOB_STATE_FINISHED 3

/**
 * @brief Internal Job Queue structure
 */
typedef struct {
    job_t *head;
    job_t *tail;

} job_queue_t;

/** @} */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Low Level Functions
 */
uint32_t __rsp_lock( uint32_t wait );
void __rsp_unlock( void );
void __rsp_wait_dma( uint32_t busy );
void __rsp_dma_read( uint32_t offs, void *src, uint32_t len );
void __rsp_dma_write( uint32_t offs, void *dst, uint32_t len );
void __rsp_blk_read( uint32_t offs, void *src, uint16_t pitch, uint16_t width, uint16_t height, uint16_t bpp );
void __rsp_blk_write( uint32_t offs, void *dst, uint16_t pitch, uint16_t width, uint16_t height, uint16_t bpp );
uint32_t __rsp_get_status( void );
void __rsp_set_status( uint32_t flags );
void __rsp_set_pc( uint32_t pc );

/**
 * @brief High Level Functions
 */
void rsp_init( void );
void rsp_close( void );
void rsp_load_lib( uint8_t *lib );
uint32_t rsp_lib_fn( char *mod, uint32_t fn );
void rsp_queue_job( job_t *job );
void rsp_do_job( job_t *job );
void rsp_wait_job( job_t *job );
void rsp_abort_job( job_t *job );
job_t *rsp_new_job( uint32_t fn, void (*cb)(job_t *), uint32_t count, ... );
void rsp_dispose_job( job_t *job );

#ifdef __cplusplus
}
#endif

#endif
