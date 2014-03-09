#ifndef CTLIB_CHAIN_ADD_H
#define CTLIB_CHAIN_ADD_H

#include "term/term_tags.h"

//exported, but, internal functionality
namespace _ctl{

//this is our equivalent of the BLAS axpy for a=1
//so its a 1xpy
template< typename Chain_iterator, typename OutputIterator, typename Term_less>
Chain_iterator chain_add( Chain_iterator x_begin, Chain_iterator x_end, 
		 	  Chain_iterator y_begin, Chain_iterator y_end,
			  OutputIterator result, const Term_less less,   
			  const _ctl::term_z2_tag t){
	//TODO: optimize set_symmetric_difference
	//TODO: Can we avoid separate return result 
	//and use x itself as the output? 
	while( true){ 
	     if(x_begin == x_end){ return std::copy( y_begin, y_end, result); }
	     if(y_begin == y_end){ return std::copy( x_begin, x_end, result); }
	     if( less(*x_begin, *y_begin)) { 
		*result = *x_begin; 
		++result; ++x_begin; 
	     }
	     //expect that Term_less compares position in filtration 
	     //when possible. No derefrence when not necessary
	     else if( less(*y_begin, *x_begin)) { 
		*result = *y_begin; 
		++result; ++y_begin;
	     } else { ++x_begin; ++y_begin; }
	}
	return result;
}

//and this is an 1xpy call for nonz2 terms
template< typename Chain_iterator, typename OutputIterator, typename Term_less>
Chain_iterator chain_add( Chain_iterator x_begin, Chain_iterator x_end, 
		  	  Chain_iterator y_begin, Chain_iterator y_end,
		  	  OutputIterator result,
			  const Term_less less,   
		  	  const _ctl::term_non_z2_tag t){
	typedef typename Chain_iterator::value_type Term;
	typedef typename Term::Coefficient Coefficient;
	
	//TODO: optimize set_symmetric_difference
	//TODO: Can we avoid separate return result 
	//and use x itself as the output? 
	while( true){ 
	     if(x_begin == x_end){ return std::copy( y_begin, y_end, result); }
	     if(y_begin == y_end){ return std::copy( x_begin, x_end, result); }
	     if( less(*x_begin, *y_begin)) { 
		*result = *x_begin; //optimize chain= operator. 
		 ++x_begin; 
	     }
	     //expect that Term_less compares position in filtration 
	     //when possible so no dereference when not necessary
	     else if( less(*y_begin,*x_begin)) { 
		*result = *y_begin; //optimize chain= operator.
	 	++y_begin;
	     } else {
		const Coefficient c = 2*x_begin->coefficient(); 
		*result = *x_begin; 
		result->coefficient( c);
		++x_begin; ++y_begin; 
	    }
	    ++result;
	}
	return result;
}

//and this is an axpy call 
//since a is in a finite field we could call it a faxpy
template< typename Chain_iterator, typename OutputIterator, typename Term_less>
Chain_iterator chain_add( Chain_iterator x_begin, Chain_iterator x_end, 
		  	  const typename Chain_iterator::value_type::
					 Coefficient& a, 
		  	  Chain_iterator y_begin, Chain_iterator y_end,
		  	  OutputIterator  result,
			  const Term_less less,   
		  	  const _ctl::term_non_z2_tag t){
	typedef typename Chain_iterator::value_type Term;
	typedef typename Term::Coefficient Coefficient;
	
	//TODO: optimize set_symmetric_difference
	//TODO: Can we avoid separate return result 
	//and use x itself as the output? 
	while( true){ 
	     if(x_begin == x_end){ return std::copy( y_begin, y_end, result); }
	     if(y_begin == y_end){ return std::copy( x_begin, x_end, result); }
	     if( less(*x_begin, *y_begin)) { 
		*result = *x_begin; //optimize chain= operator. 
		 ++x_begin; 
	     }
	     //expect that Term_less compares position in filtration 
	     //when possible. No derefrence when not necessary
	     else if( less(*y_begin,*x_begin)) { 
		const Coefficient c = a*y_begin->coefficient(); 
		if( c == 0 ) { continue; } 
		*result = *y_begin; //optimize chain= operator.
		result->coefficient( c);
	 	++y_begin;
	     } else {
		const Coefficient c = x_begin->coefficient() + 
				    a*y_begin->coefficient();
		if( c == 0 ) { continue; } 
		*result = *x_begin; 
		result->coefficient( c);
		++x_begin; ++y_begin; 
	    }
	    ++result;
	}
	return result;
}

//over Z2 the only coeff is 1
template< typename Chain, typename Term>
Chain& chain_add( Chain & x, Term & y, const _ctl::term_z2_tag t ){
	typedef typename Chain::Less Less;
	Less less;
	auto pos = std::lower_bound( x.begin(), x.end(), y);
	//new element, so add it at the end
	if (pos == x.end()) { x.push_back( y); }
	//new element in the middle, add it.
	else if (less(y, *pos)){ x.insert( pos, y); }
	//element exists, in Z2 this means delete!
	else{ x.erase( pos); }
	return x;
}

template< typename Term>
struct is_zero{
	typedef typename Term::Coefficient Coefficient;
	const bool operator()( const Term & t) const{
		return (t.coefficient() == 0);
	}
}; //struct is_zero

//Since x=x+a*y is well defined for y a term we ignore the coefficient
template< typename Chain, typename Term>
Chain& chain_add( Chain & x, const Term & y, const _ctl::term_non_z2_tag t){
	typedef typename Chain::Less Less;
	Less less;
	typedef typename Term::Coefficient Coefficient;
	typedef _ctl::is_zero< Term> Is_zero;
	auto pos = std::lower_bound( x.begin(), x.end(), y);
	//new element, so add it at the end
	if (pos == x.end()) { x.push_back( y); }
	//new element in the middle, add it.
	else if (less(y,*pos)){ x.insert( pos, y); }
	//element exists, but adding y might delete it!
	else{
	 pos->coefficient( y.coefficient(), true);
	 x.erase( std::remove_if( pos, pos+1, Is_zero()));	
	}
	return x;
}

} //namespace _ctl

#endif //CTLIB_CHAIN_ADD_H