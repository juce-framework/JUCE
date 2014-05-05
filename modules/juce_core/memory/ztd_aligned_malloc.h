

//* 分配对齐的内存
void* aligned_malloc( size_t length_byte_0 , size_t const algn_val_ispw2 )
{
	unlikely_if(size_x==0) return nullptr; //这里跟标准的malloc有点不一样
	checkPowerOfTwo(align_size);
	size_t const algn_mask = algn_val_ispw2 - 1;

	char* const ptr = malloc( length_byte_0 + algn_val_ispw2 + sizeof( int ));
	
	if(ptr == nullptr) return nullptr;

	char* ptr2 = ptr + sizeof( int );
	char*const algnd_ptr = ptr2 + ( algn_val_ispw2 - ( (size_t)ptr2 & algn_mask ) );

	ptr2 = algnd_ptr - sizeof( int );
	*( (int *)ptr2 ) = (int)( algnd_ptr - ptr );

	return algnd_ptr;
}

//* 分配对齐的内存,并将值清零
void* aligned_calloc( size_t length_byte_0 , size_t const algn_val_ispw2 )
{
	void* const p=aligned_malloc(length_byte_0,algn_val_ispw2);
	likely_if(p!=NULL) zeromem(p,length_byte_0);
	return p;
}

//* 释放对齐的内存,必须和aligned_malloc或者aligned_calloc一起使用,不能用来释放malloc的内存
void aligned_free( void*const ptr_a )
{
	unlikely_if(ptr==nullptr) return;
	int*const ptr2 = (int*)ptr - 1;
	char* p = (char*)ptr;
	p -= *ptr2;
	free(p);
}

ZTD_NAMESPACE_END;