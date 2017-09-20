#pragma once
/* 
 * ------------------------------------------------------------------------------------------------
 * file:		VerticalExt.h
 * version:		1.0
 * author:		dirk.van-maercke@cstb.fr
 * copyright:	see file licence.EU.txt
 * description: data structures for vertical extensions associated with propagation paths
 * changes:
 *
 *	18/01/2013	initial version
 *
 *  13/11/2013	definition of elementary sources moved to a separate file (ElementarySource.h)
 *
 *	13/11/2013	type of extension made a private member of the Extension class
 *
 * ------------------------------------------------------------------------------------------------- 
 */
#include "ReferenceObject.h"
#include "Material.h"
#include "SourceGeometry.h"
#include "ElementarySource.h"

namespace CnossosEU
{
	/*
	 * VerticalExt abstract base class
	 */
	struct VerticalExt : public System::ReferenceObject
	{
		enum VerticalExtType
		{
			Undefined = 0,
			Source = 1,
			Receiver = 2,
			Barrier = 3,
			VerticalWall = 4,
			VerticalEdge = 5
		} ;
	public:
		/*
		 * all extensions have some height above the local boundary
		 */
		double h ;
		/*
		 * constructor
		 */
		VerticalExt (VerticalExtType _type, double _h = 0.0) : type(_type), h(_h) { }
		/*
		 * abstract base classes cannot be copied but may support cloning
		 */
		virtual VerticalExt* clone (void) = 0 ;
		/*
		 * get extension type
		 */
		bool isSource (void) { return type == Source ; }
		bool isReceiver (void) { return type == Receiver ; }
		bool isBarrier (void) { return type == Barrier ; }
		bool isVerticalWall (void) { return type == VerticalWall ; }
		bool isVerticalEdge (void) { return type == VerticalEdge ; }

	protected:
		/*
		 * copy construct is protected (for use in derived classes only)
		 */
		VerticalExt (VerticalExt const& other) : type (other.type), h (other.h) { } ;
	
	private:
		/*
		 * assignment operator is not implemented
		 */
		VerticalExt& operator= (VerticalExt const&) ;

	private:
		/*
		 * extension type allows for fast identification of non-abstract extensions
		 */
		VerticalExtType  type ;
	};
	/*
	 * generic clone operation
	 */
	#define IMPLEMENT_CLONE(TYPE) TYPE* clone(void) { return new TYPE (*this) ; }
	/*
	 * Abstract source extension
	 */
	struct SourceExt : public VerticalExt
	{
		/*
		 * extended source geometry for point, line or area sources
		 */
		System::ref_ptr<SourceGeometry> geo ;
		/*
		 * elementary source defines the source in terms of sound power and directivity
		 *
		 * note that a real application would support multiple elementary sources, i.e. equivalent
		 * point sources at different heights, with different sound powers and directivity functions.
		 * 
		 * in order to implement multiple source heights, replace the following attribute by:
		 * 
		 *		std::vector<ElementarySource> sources
		 */
		ElementarySource source ;

		SourceExt (void) 
		: VerticalExt (VerticalExt::Source), source(), geo() { } ;
		
		SourceExt (double _h) 
		: VerticalExt (VerticalExt::Source, _h), source(), geo() { } ;

		SourceExt (ElementarySource const& _source, SourceGeometry* _geo = NULL) 
		: VerticalExt (VerticalExt::Source), source(_source), geo(_geo) 
		{
			//h = source.sourceHeight ;
		} ;

		SourceExt (SourceExt const& _other) : VerticalExt (_other), source(_other.source), geo()
		{
			if (_other.geo) geo = _other.geo->clone() ;
		} ;
		IMPLEMENT_CLONE (SourceExt) ;
	};
	/*
	 * Receiver extension
	 */
	struct ReceiverExt : public VerticalExt
	{
		ReceiverExt (void) : VerticalExt (VerticalExt::Receiver)
		{ } ;
		ReceiverExt (ReceiverExt const& other) : VerticalExt (other) 
		{ } ;
		ReceiverExt (double _h) : VerticalExt (VerticalExt::Receiver, _h)	
		{ } ;
		IMPLEMENT_CLONE (ReceiverExt) ;
	};
	/*
	 * Barrier extension
	 */
	struct BarrierExt : public VerticalExt
	{
		System::ref_ptr<Material> mat ;
		BarrierExt (void) : VerticalExt (VerticalExt::Barrier) 
		{
			mat = getMaterial ("A0") ;
		} ;
		BarrierExt (double _h, Material* _mat = 0) : VerticalExt (VerticalExt::Barrier, _h), mat (_mat)
		{
			if (mat == 0) mat = getMaterial ("A0") ;
		} ;
		BarrierExt (double _h, const char* _mat) : VerticalExt (VerticalExt::Barrier, _h)
		{
			mat = getMaterial (_mat) ;
		} ;
		BarrierExt (BarrierExt const& _other) : VerticalExt (_other), mat (_other.mat) 
		{ } ;
		IMPLEMENT_CLONE (BarrierExt) ;
	};
	/*
	 * Vertical wall extension
	 */
	struct VerticalWallExt : public VerticalExt
	{
		System::ref_ptr<Material> mat ;
		VerticalWallExt (void) : VerticalExt (VerticalExt::VerticalWall) 
		{
			mat = getMaterial ("A0") ;
		} ;
		VerticalWallExt (double _h, Material* _mat = 0) : VerticalExt (VerticalExt::VerticalWall, _h), mat(_mat)
		{
			if (mat == 0) mat = getMaterial ("A0") ;
		} ;
		VerticalWallExt (double _h, const char* _mat) : VerticalExt (VerticalExt::VerticalWall, _h) 
		{
			mat = getMaterial (_mat) ;
		} ;
		VerticalWallExt (VerticalWallExt const& other) : VerticalExt (other), mat (other.mat) { } ;
		IMPLEMENT_CLONE (VerticalWallExt) ;
	};
	/*
	 * Vertical edge extension
	 */
	struct VerticalEdgeExt : public VerticalExt
	{
		VerticalEdgeExt (void) : VerticalExt (VerticalExt::VerticalEdge)
		{ } ;
		VerticalEdgeExt (double _h) : VerticalExt (VerticalExt::VerticalEdge, _h)
		{ } ;
		VerticalEdgeExt (VerticalEdgeExt const& _other) : VerticalExt (_other) 
		{ } ;
		IMPLEMENT_CLONE (VerticalEdgeExt) ;
	};
}