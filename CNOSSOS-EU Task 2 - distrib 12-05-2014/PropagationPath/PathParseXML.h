#pragma once
/* 
 * ------------------------------------------------------------------------------------------------
 * file:		PathParseXML.h
 * version:		1.0
 * author:		dirk.van-maercke@cstb.fr
 * copyright:	see file licence.EU.txt
 * description: parse input to the propagation path calculator from XML file
 * changes:
 *
 *	18/01/2013	initial version
 * ------------------------------------------------------------------------------------------------- 
 */
#include "../SimpleXMl/SimpleXML.h"
#include "./PropagationPath.h"
#include "./CalculationMethod.h"
#include "./ErrorMessage.h"

namespace CnossosEU
{
	/*
	 * error handling: invalid XML file syntax
	 */
	class XMLSyntaxError : public ErrorMessage
	{
		unsigned int last_line ;
		unsigned int last_char ;
		unsigned int last_error ;
	public:
		XMLSyntaxError (XMLFileParser& p) : ErrorMessage ()
		{
			last_line  = p.last_line ;
			last_char  = p.last_char ;
			last_error = p.last_error ;
		}
		virtual void print (void)
		{
			if (last_error == XML_FILE_NOT_FOUND)
			{
				printf ("ERROR: file not found\n") ;
			}
			else if (last_error == XML_FILE_NO_ACCESS)
			{
				printf ("ERROR: file cannot be read (access denied)\n") ;
			}
			else
			{
				printf ("ERROR: syntax error while parsing file\n") ;
				printf (".internal error code = %s\n", XMLFileParser::GetErrorText(last_error)) ;
				printf (".at position (line %u, column %u)\n", last_line, last_char) ;
			}
		}
		virtual const char* what (void) const throw()
		{
			if (last_error == XML_FILE_NOT_FOUND)
			{
				return "file not found" ;
			}
			else if (last_error == XML_FILE_NO_ACCESS)
			{
				return "file cannot be read (access denied)" ;
			}
			else
			{
				return "syntax error while parsing file" ;
			}
		}
	};
	/*
	 * error handling: invalid XML file semantics
	 */
	class XMLParseError : public ErrorMessage
	{
		XMLNode* _node ;
	
	protected:

		void print_context (void)
		{
			if (!_node) return ;
			printf (".while parsing tag <%s> \n", _node->GetName()) ;
			XMLNode* parent = _node->GetParent() ;
			while (parent != 0)
			{				
				printf (".from parent <%s", parent->GetName()) ;
				const char* id = parent->GetAttribute ("id") ;
				if (id != 0) printf (" id=\"%s\"", id) ;
				printf (">\n") ;
				parent = parent->GetParent() ;
			}
		}

	public:

		XMLParseError (const char* what, XMLNode* node) : ErrorMessage (what), _node(node) { }
		XMLNode* node (void) const { return _node ; }
		
		virtual void print (void)
		{
			printf ("ERROR: %s \n", what()) ;
			if (_node != 0)
			{
				print_context() ;		
			}
		}
	};
	/*
	 * error handling: unexpected tag detected
	 */
	class XMLUnexpectedTag : public XMLParseError
	{
	public:
		XMLUnexpectedTag (XMLNode* node) : XMLParseError ("unexpected tag", node) { }
	};
	/*
	 * error handling: missing tag detected
	 */
	class XMLMissingTag : public XMLParseError
	{
		const char* _tag ;
	
	public:
	
		XMLMissingTag (const char* tag, XMLNode* node) : XMLParseError ("missing tag", node), _tag (tag) { }
		const char* tag (void) { return _tag ; }
		
		virtual void print (void)
		{
			printf ("ERROR: %s \n", what()) ;
			printf (".expecting tag <%s> \n", tag()) ;
			print_context() ;
		}
	};
	/*
	 * load and parse file 
	 */
	bool ParsePathFromFile (XMLNode* root, PropagationPath& path, PropagationPathOptions& options) ;
}
