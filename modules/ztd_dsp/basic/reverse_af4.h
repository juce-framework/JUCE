#ifndef memory_h__
#define memory_h__

inline void memset_af4( float*const restrict ptr_a , const size_t length_4 , float value );

inline void memzero_af4( float*const restrict ptr_a , const size_t length_4 );

inline void memmove_af4( float*const opt_a , float*const ipt_a , const size_t length_4 );

inline void reverse_af4( float*const ptr_a , const size_t length_8 );


#endif // memory_h__
