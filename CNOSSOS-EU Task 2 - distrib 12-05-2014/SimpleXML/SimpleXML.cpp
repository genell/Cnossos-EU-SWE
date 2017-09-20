/* 
 * ------------------------------------------------------------------------------------------------
 * file:		SimpleXML.cpp
 * version:		1.001
 * author:		dirk.van-maercke@cstb.fr
 * copyright:	see file licence.CSTB.txt
 * description: SimpleXML defines a light-weight XML-to-DOM parser developed by CSTB.  
 * dependency:  SimpleXML builds on top of the EXPAT event-driven XMLparser, an open-source library 
 *			    available under LGPL from http://expat.sourceforge.net/
 * changes:
 *
 *	16/10/2013	this notice included
 *
 *	15/11/2013	support added to convert numerical error codes to human-readable text
 *
 * ------------------------------------------------------------------------------------------------- 
 */
#include "./SimpleXML.h"
#ifdef __GNUC__
#define strcat_s strcat
#define strncpy_s strncpy
#endif
// ------------------------------------------------------------------------------------------------
// interface avec EXPAT
// ------------------------------------------------------------------------------------------------

static void start_handler(void *userData, const char *elementName, const char **attributes) 
{
	XMLFileParser *doc = (XMLFileParser*) userData ;    
	doc->startEntity (elementName, attributes) ;
}

static void end_handler (void* userData, const char *elementName) 
{
	XMLFileParser *doc = (XMLFileParser*) userData ;    
	doc->endEntity (elementName) ;
} 

static void text_handler (void *userData, const XML_Char *text, int len_text)
{
	int i;
	for (i = 0 ; i < len_text ; i++) if (text[i] > ' ') break ;
	if (i == len_text) return ;
	if (i > 0) i-- ;
	XMLFileParser *doc = (XMLFileParser*) userData ;    
	doc->addText(text+i, len_text-i) ;
}

// ------------------------------------------------------------------------------------------------
// lecture du fichier et interprétation par EXPAT
//
// A FAIRE: gestion des erreurs
// ------------------------------------------------------------------------------------------------

bool XMLFileParser::ParseFile (const char* fileName)
{
	const int BUFF_SIZE = 4096 ;

	XML_Parser p = XML_ParserCreate(NULL) ;

	XML_SetUserData (p, (void*) this) ;

	XML_SetElementHandler(p, start_handler, end_handler) ;
	XML_SetCharacterDataHandler(p, text_handler) ;

	last_line = 0 ;
	last_char = 0 ;
	last_error = 0 ;

	if (fileName == 0 || _access (fileName,0) != 0)
	{
		last_error = XML_FILE_NOT_FOUND ;
		return false ;
	}

	int file_id = _open (fileName, O_RDONLY | O_BINARY) ;
	if (file_id < 0)
	{
		last_error = XML_FILE_NO_ACCESS ;
		return false ;
	}

	while (true)
	{
		void *buff = XML_GetBuffer(p, BUFF_SIZE);
		if (buff == NULL) 
		{
			last_error = XML_GetErrorCode (p) ;
			break ;
		}

		int bytes_read = _read (file_id, buff, BUFF_SIZE);

		if (bytes_read == 0) break;

		if (bytes_read < 0) 
		{
			last_error = XML_GetErrorCode(p) ;
			break ;
		}

		if (! XML_ParseBuffer(p, bytes_read, bytes_read == 0)) 
		{
			last_error = XML_GetErrorCode (p) ;
			break ; 
		}
	}  

	last_line = XML_GetCurrentLineNumber (p) ;
	last_char = XML_GetCurrentColumnNumber (p) ;

	XML_ParserFree (p) ;

	return (last_error == XML_ERROR_NONE) ;
}

// ------------------------------------------------------------------------------------------------
// affichage à l'écran du contenu d'un fichier XML 
// ------------------------------------------------------------------------------------------------

void XMLDumpFile::startEntity (const char *el, const char **attr) 
{

	int i;

	printf("%s<%s> \n", depth, el);

	for (i = 0; attr[i]; i += 2) 
	{
		printf("%s .%s = '%s'\n", depth,attr[i], attr[i + 1]);
	}
	strcat_s(depth," ") ;
}

void XMLDumpFile::endEntity (const char *el) 
{
	size_t n = strlen(depth) ;
	depth[n-1] = 0 ;
	printf("%s</%s> \n", depth, el);
} 

void XMLDumpFile::addText(const XML_Char *s, int len)
{
	char buffer[1024] ;
	strncpy_s(buffer, s, len) ;
	buffer[len] = 0 ;
	printf ("%s.text = '%s'\n",depth,buffer) ; 
}

// ------------------------------------------------------------------------------------------------
// utilitaires pour manipuler des chaînes de caractères
// ------------------------------------------------------------------------------------------------

static char* copy_text (const char* text, int len = -1)
{
	if (len < 0) len = (int) strlen(text) ;
	char* s = (char*) malloc((len+1) * sizeof(char)) ;
	strncpy (s, text, len) ;
	s[len] = 0 ;
	return s ;
}

static char* append_text (char *init, const char* text, int len = -1)
{
	if (init == NULL) return copy_text (text, len) ;

	if (len < 0) len = (int) strlen(text) ;
	size_t init_len = strlen (init) ;
	char* s = (char*) malloc((init_len + len + 1) * sizeof(char)) ;
	strncpy (s, init, init_len) ;
	strncpy (s+init_len, text, len) ;
	s[init_len+len] = 0 ;
	free (init) ;
	return s ;
}

static char** copy_attrib (const char** attrib) 
{
	int n;
	for (n = 0 ; attrib[n] ; n+=2) ;
	char** list = (char**) malloc((n+1) * sizeof(char*)) ;

	int i;
	for (i = 0 ; i < n ; i++)
	{
		list[i] = copy_text (attrib[i]) ;
	}
	list[i] = NULL ;
	return list ;
}

// ------------------------------------------------------------------------------------------------
// représentation d'un document XML sous la forme d'une arborescence
// chaque noeud correspond à une entité du fichier XML
// ------------------------------------------------------------------------------------------------

void XMLNode::addText (const char* new_text, int len_text)
{
	text = append_text (text, new_text, len_text) ;
}   

XMLNode::XMLNode (XMLNode *_parent, const char* _name, const char** _attrib)
{
	parent      = _parent ;
	next        = NULL ;
	first_child = NULL ;
	last_child  = NULL ;
	name        = copy_text(_name) ;
	text        = NULL ;
	attrib      = copy_attrib(_attrib) ;

	if (parent)
	{
		if (parent->last_child)
		{
			parent->last_child->next = this ;
			parent->last_child = this ;
		}
		else
		{
			parent->first_child = this ;
			parent->last_child  = this ;
		}    
	}
}

XMLNode* XMLNode::GetChild (const char *_name) 
{
	XMLNode *c = first_child ;
	while (c)
	{
		if (strcmp(c->name, _name) == 0) return c ;
		c = c->next ;
	}
	//printf ("ERROR:could not find entity [%s] from [%s] \n", _name, name) ;
	return NULL ;
}


const char* XMLNode::GetAttribute (const char *_name) 
{
	for (int i = 0; attrib[i] ; i+=2)
	{
		if (strcmp(attrib[i],_name) == 0) return attrib[i+1] ;
	}
	//printf ("ERROR:could not find attribute [%s] from [%s] \n", _name, name) ;
	return NULL ;
}

XMLNode* XMLNode::GetChildByAttribute (const char *_attrib, const char *_value)
{
	XMLNode *c = first_child ;
	while (c)
	{
		for (int i = 0 ; c->attrib[i] ; i += 2)
		{
			if ((strcmp(c->attrib[i], _attrib) == 0) &&
				(strcmp(c->attrib[i+1], _value) == 0)) return c ;
		}
		c = c->next ;
	}
	//printf ("ERROR:could not find attribute [%s=%s] from [%s] \n", _attrib, _value, name) ;
	return NULL ;    
}

void XMLNode::dump (int level)
{
	for (int i = 0 ; i < level ; i++) printf (" ") ;
	printf ("<%s>",name) ;        
	printf ("\n") ;

	for (int j = 0; attrib[j]; j += 2) 
	{
		for (int i = 0 ; i < level ; i++) printf (" ") ;
		printf(".%s = '%s'", attrib[j], attrib[j + 1]);
		printf ("\n") ;
	}

	if (text)
	{
		for (int i = 0 ; i < level ; i++) printf (" ") ;
		printf (".text = '%s'",text) ;
		printf ("\n") ;
	}

	XMLNode *child = first_child ;
	while (child)
	{
		child->dump (level+1) ;
		child = child->next ;
	}

	for (int i = 0 ; i < level ; i++) printf (" ") ;
	printf ("</%s>",name) ;
	printf ("\n") ;
}


XMLNode::~XMLNode (void)
{
	XMLNode *child = first_child ;
	while (child)
	{
		XMLNode *next = child->next ;
		delete child ;
		child = next ;
	}

	if(attrib)
	{
		for (int i = 0 ; attrib[i] != NULL; i += 2)
		{
			free (attrib[i]) ;
			free (attrib[i+1]) ;
		}

		free (attrib) ;	
	}

	if(name) free (name) ;
	if(text) free(text) ;
}

// ------------------------------------------------------------------------------------------------
// transformation d'un document XML en une arborescence de XMLNode
// ------------------------------------------------------------------------------------------------

void XMLFileLoader::startEntity (const char *element_name, const char **attr) 
{
	XMLNode* node = new XMLNode (current, element_name, attr) ;
	if (!root) root = node ;
	current = node ;
}

void XMLFileLoader::endEntity (const char *element_name) 
{
	current = current->GetParent() ;
}

void XMLFileLoader::addText (const XML_Char* text, int len_text) 
{
	current->addText (text, len_text) ;
}

// ------------------------------------------------------------------------------------------------
// transformation d'un code d'erreur numérique en une chaîne de caractères correspondante
// ------------------------------------------------------------------------------------------------

#define _ERROR_TEXT_(x) {x, #x }
static struct 
{
	unsigned int code ;
	const char* text ;
} xml_error_text[] = 
{
	_ERROR_TEXT_(XML_ERROR_NO_MEMORY),
	_ERROR_TEXT_(XML_ERROR_SYNTAX),
	_ERROR_TEXT_(XML_ERROR_NO_ELEMENTS),
	_ERROR_TEXT_(XML_ERROR_INVALID_TOKEN),
	_ERROR_TEXT_(XML_ERROR_UNCLOSED_TOKEN),
	_ERROR_TEXT_(XML_ERROR_PARTIAL_CHAR),
	_ERROR_TEXT_(XML_ERROR_TAG_MISMATCH),
	_ERROR_TEXT_(XML_ERROR_DUPLICATE_ATTRIBUTE),
	_ERROR_TEXT_(XML_ERROR_JUNK_AFTER_DOC_ELEMENT),
	_ERROR_TEXT_(XML_ERROR_PARAM_ENTITY_REF),
	_ERROR_TEXT_(XML_ERROR_UNDEFINED_ENTITY),
	_ERROR_TEXT_(XML_ERROR_RECURSIVE_ENTITY_REF),
	_ERROR_TEXT_(XML_ERROR_ASYNC_ENTITY),
	_ERROR_TEXT_(XML_ERROR_BAD_CHAR_REF),
	_ERROR_TEXT_(XML_ERROR_BINARY_ENTITY_REF),
	_ERROR_TEXT_(XML_ERROR_ATTRIBUTE_EXTERNAL_ENTITY_REF),
	_ERROR_TEXT_(XML_ERROR_MISPLACED_XML_PI),
	_ERROR_TEXT_(XML_ERROR_UNKNOWN_ENCODING),
	_ERROR_TEXT_(XML_ERROR_INCORRECT_ENCODING),
	_ERROR_TEXT_(XML_ERROR_UNCLOSED_CDATA_SECTION),
	_ERROR_TEXT_(XML_ERROR_EXTERNAL_ENTITY_HANDLING),
	_ERROR_TEXT_(XML_ERROR_NOT_STANDALONE),
	_ERROR_TEXT_(XML_ERROR_UNEXPECTED_STATE),
	_ERROR_TEXT_(XML_ERROR_ENTITY_DECLARED_IN_PE),
	_ERROR_TEXT_(XML_ERROR_FEATURE_REQUIRES_XML_DTD),
	_ERROR_TEXT_(XML_ERROR_CANT_CHANGE_FEATURE_ONCE_PARSING),
	_ERROR_TEXT_(XML_ERROR_UNBOUND_PREFIX),
	_ERROR_TEXT_(XML_ERROR_UNDECLARING_PREFIX),
	_ERROR_TEXT_(XML_ERROR_INCOMPLETE_PE),
	_ERROR_TEXT_(XML_ERROR_XML_DECL),
	_ERROR_TEXT_(XML_ERROR_TEXT_DECL),
	_ERROR_TEXT_(XML_ERROR_PUBLICID),
	_ERROR_TEXT_(XML_ERROR_SUSPENDED),
	_ERROR_TEXT_(XML_ERROR_NOT_SUSPENDED),
	_ERROR_TEXT_(XML_ERROR_ABORTED),
	_ERROR_TEXT_(XML_ERROR_FINISHED),
	_ERROR_TEXT_(XML_ERROR_SUSPEND_PE),
	_ERROR_TEXT_(XML_ERROR_RESERVED_PREFIX_XML),
	_ERROR_TEXT_(XML_ERROR_RESERVED_PREFIX_XMLNS),
	_ERROR_TEXT_(XML_ERROR_RESERVED_NAMESPACE_URI),
	_ERROR_TEXT_(XML_ERROR_NONE)
};

const char* XMLFileParser::GetErrorText (unsigned int code)
{
	for (unsigned int pos = 0 ; xml_error_text[pos].code != XML_ERROR_NONE ; ++pos)
	{
		if (xml_error_text[pos].code == code) return xml_error_text[pos].text ;
	}
	return "XML_UNKOWN_ERROR" ;
}