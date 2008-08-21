#include "ticpp.h"
#include <boost/shared_ptr.hpp>

namespace DBBinder
{

// XML Stuff
typedef ticpp::Document XMLDocument;
typedef XMLDocument* XMLDocumentPtr;
typedef boost::shared_ptr<XMLDocument> XMLDocumentSPtr;

typedef ticpp::Element XMLElement;
typedef XMLElement* XMLElementPtr;

typedef ticpp::Node XMLNode;
typedef XMLNode* XMLNodePtr;

typedef ticpp::Attribute XMLAttribute;
typedef XMLAttribute* XMLAttributePtr;

}
