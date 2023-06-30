//////////////////////////////////////////////////////////////////////
//
//	Variable-length Vector header
//	
//	File: vectorn.h
//	Description : vectorn template class declaration and inlined implementation
//
//	History:
//	-:Created by Anton Knyazev
//
//////////////////////////////////////////////////////////////////////

#ifndef vectorn_h
#define vectorn_h
#pragma once

template <class ftype> class matrix_vector_product_tpl {
public:
	matrix_vector_product_tpl(int nrows,int ncols,ftype *pmtxdata, ftype *pvecdata) {
		nRows=nrows; nCols=ncols; mtxdata=pmtxdata; vecdata=pvecdata;
	}
	int nRows,nCols;
	ftype *mtxdata,*vecdata;
};

template <class ftype> class vectorn_tpl {
public:
	vectorn_tpl(int _len,ftype *pdata=0) { 
		len = _len;
		if (pdata) { flags=mtx_foreign_data; data=pdata; }
#ifdef USE_MATRIX_POOLS
		else if (len<64) {
			if (vecn_pool_pos+len > vecn_pool_size)
				vecn_pool_pos = 0;
			data = vecn_pool+vecn_pool_pos;
			vecn_pool_pos += len;
			flags = mtx_foreign_data;
		} 
#endif
		else { data=new ftype[len]; flags=0; }
	}
	vectorn_tpl(vectorn_tpl &src) { flags=src.flags & mtx_foreign_data; data=src.data; len=src.len; src.flags |= mtx_foreign_data; };
	~vectorn_tpl() { if (!(flags & mtx_foreign_data)) delete[] data; }

	vectorn_tpl& operator=(const vectorn_tpl<ftype> &src) {
		if (src.len!=len && !(flags & mtx_foreign_data)) {
			if (data) delete data; 
			data = new ftype[src.len];
		}
		len = src.len;
		for(int i=0;i<len;i++) data[i]=src.data[i];
		return *this;
	}
	template<class ftype1> vectorn_tpl& operator=(const vectorn_tpl<ftype1> &src) {
		if (src.len!=len && !(flags & mtx_foreign_data)) {
			if (data) delete data; 
			data = new ftype[src.len];
		}
		len = src.len;
		for(int i=0;i<len;i++) data[i]=src.data[i];
		return *this;
	}

	vectorn_tpl& operator=(const matrix_vector_product_tpl<ftype> &src) {
		int i,j;
		for(i=0;i<src.nRows;i++) for(data[i]=0,j=0;j<src.nCols;j++)
			data[i] += src.mtxdata[i*src.nCols+j]*src.vecdata[j];
		return *this;
	}
	vectorn_tpl& operator+=(const matrix_vector_product_tpl<ftype> &src) {
		int i,j; 
		for(i=0;i<src.nRows;i++) for(j=0;j<src.nCols;j++)
			data[i] += src.mtxdata[i*src.nCols+j]*src.vecdata[j];
		return *this;
	}
	vectorn_tpl& operator-=(const matrix_vector_product_tpl<ftype> &src) {
		int i,j; 
		for(i=0;i<src.nRows;i++) for(j=0;j<src.nCols;j++)
			data[i] -= src.mtxdata[i*src.nCols+j]*src.vecdata[j];
		return *this;
	}

	ftype len2() { ftype res=0; for(int i=0;i<len;i++) res += data[i]*data[i]; return res; }

	ftype& operator[](int idx) const { return data[idx]; }
	operator ftype*() { return data; }
	vectorn_tpl& zero() { for(int i=0;i<len;i++) data[i]=0; return *this; }
	vectorn_tpl& allocate() { 
		if (flags & mtx_foreign_data) {
			ftype *newdata = new ftype[len];
			for(int i=0;i<len;i++) newdata[i]=data[i];
			data=newdata; flags&=~mtx_foreign_data;
		}
	}

	vectorn_tpl operator*(ftype op) const {
		vectorn_tpl<ftype> res(len);
		for(int i=0;i<len;i++) res.data[i] = data[i]*op;
		return res;
	}

	vectorn_tpl& operator*=(ftype op) {
		for(int i=0;i<len;i++) data[i] *= op;
		return *this;
	}

	ftype *data;
	int len;
	int flags;

#ifdef USE_MATRIX_POOLS
	static ftype vecn_pool[];
	static int vecn_pool_pos;
	static int vecn_pool_size;
#endif
};

template<class ftype1,class ftype2>
ftype1 operator*(const vectorn_tpl<ftype1>& op1, const vectorn_tpl<ftype2>& op2) {
	ftype1 res=0; for(int i=0;i<op1.len;i++) res += op1.data[i]*op2.data[i];
	return res;
}

/*template<class ftype1,class ftype2>
vectorn_tpl<ftype1> operator+(const vectorn_tpl<ftype1> &op1,const vectorn_tpl<ftype2> &op2) {
	vectorn_tpl<ftype1> res(len);
	for(int i=0;i<len;i++) res.data[i] = op1.data[i]+op2.data[i];
	return res;
}
*/
template<class ftype1,class ftype2>
vectorn_tpl<ftype1> operator-(const vectorn_tpl<ftype1> &op1, const vectorn_tpl<ftype2> &op2) {
	vectorn_tpl<ftype1> res(op1.len);
	for(int i=0;i<op1.len;i++) res.data[i] = op1.data[i]-op2.data[i];
	return res;
}

template<class ftype1,class ftype2>
vectorn_tpl<ftype1>& operator+=(vectorn_tpl<ftype1> &op1, const vectorn_tpl<ftype2> &op2) {
	for(int i=0;i<op1.len;i++) op1.data[i] += op2.data[i];
	return op1;
}

template<class ftype1,class ftype2>
vectorn_tpl<ftype1>& operator-=(vectorn_tpl<ftype1> &op1, const vectorn_tpl<ftype2> &op2) {
	for(int i=0;i<op1.len;i++) op1.data[i]-=op2.data[i];
	return op1;
}

template<class ftype1,class ftype2>
vectorn_tpl<ftype1> operator*(const vectorn_tpl<ftype1> &vec, const matrix_tpl<ftype2> &mtx) {
	int i,j; vectorn_tpl<ftype1> res(mtx.nCols);
	for(i=0;i<mtx.nCols;i++) for(res.data[i]=0,j=0;j<vec.len;j++)
		res.data[i] += vec[j]*mtx[j][i];
	return res;
}

/*template<class ftype1,class ftype2> 
vectorn_tpl<ftype1> operator*(const matrix_tpl<ftype1> &mtx, const vectorn_tpl<ftype2> &vec) {
	int i,j; vectorn_tpl<ftype1> res(mtx.nRows); 
	for(i=0;i<mtx.nRows;i++) for(res.data[i]=0,j=0;j<vec.len;j++)
		res.data[i] += mtx[i][j]*vec[j];
	return res;
}*/

template<class ftype> 
matrix_vector_product_tpl<ftype> operator*(const matrix_tpl<ftype> &mtx, const vectorn_tpl<ftype> &vec) {
	return matrix_vector_product_tpl<ftype>(mtx.nRows,mtx.nCols,mtx.data, vec.data);
}


typedef vectorn_tpl<float> vectornf;
typedef vectorn_tpl<real> vectorn;

#if defined(LINUX)
	#define DECLARE_VECTORN_POOL(ftype,sz) template<> ftype vectorn_tpl<ftype>::vecn_pool[sz] = {}; template<> int vectorn_tpl<ftype>::vecn_pool_pos=0; \
		template<> int vectorn_tpl<ftype>::vecn_pool_size=sz;
#else
	#define DECLARE_VECTORN_POOL(ftype,sz) template<> ftype vectorn_tpl<ftype>::vecn_pool[sz]; template<> int vectorn_tpl<ftype>::vecn_pool_pos=0; \
		template<> int vectorn_tpl<ftype>::vecn_pool_size=sz;
#endif //LINUX

#endif