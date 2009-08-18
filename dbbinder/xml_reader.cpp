/*
    Copyright 2008 Gianni Rossi

    This file is part of DBBinder++.

    DBBinder++ is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    DBBinder++ is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with DBBinder++.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "TinyXML/nvXML.h"

#include "main.h"
#include "xml_reader.h"

namespace DBBinder
{

static String fileName;

static void getXMLParams(XMLElementPtr _elem, AbstractElements* _elements)
{
	{
		XMLElementPtr sql = _elem->FirstChildElement("sql");
		if ( sql )
			_elements->sql = sql->GetText();
		else
		{
			sql = _elem->FirstChildElement("include");
			if ( sql )
			{
				String path( getFilenameRelativeTo(fileName, sql->GetText()) );

				std::ifstream include(path.c_str());
				if ( include.good() )
				{
					include.seekg(0, std::ios_base::end);
					int size = include.tellg();
					include.seekg(0);

					char *buffer = static_cast<char*>( malloc( size + 1 ) );

					include.read(buffer, size);
					buffer[size] = '\0';

					_elements->sql = buffer;

					free( buffer );
				}
			}
		}
	}

	SQLTypes type;
	String name, defaultValue, strType;
	int index;

	XMLElementPtr param;
	XMLNodePtr node = 0;
	while( node = _elem->IterateChildren( "param", node ))
	{
		param = node->ToElement();

		param->GetAttribute( "name", &name );
		param->GetAttributeOrDefault( "type", &strType, "" );
		param->GetAttributeOrDefault( "default", &defaultValue, "" );
		param->GetAttributeOrDefault( "index", &index, -1 );

		type = typeNameToSQLType(strType);
		if ( type == stUnknown )
		{
			WARNING("unknown param type for: " << name);
			type = stText;
		}

		_elements->input.push_back( SQLElement( name, type, index, defaultValue ));
	}
}

void parseXML(const String& _fileName, AbstractGenerator **_generator)
{
	fileName = _fileName;

	try
	{
		XMLDocument xmlFile( fileName );
		xmlFile.LoadFile();

		XMLElementPtr xml = xmlFile.FirstChildElement("xml");

		XMLElementPtr db = xml->FirstChildElement("database");

		if ( !*_generator )
			*_generator = AbstractGenerator::getGenerator( db->FirstChildElement("type")->GetText() );

		XMLElementPtr elem;
		XMLNodePtr node, subnode, valnode;

		node = 0;
		while( node = db->IterateChildren( node ))
		{
			elem = node->ToElement();

			String str;
			elem->GetAttribute( "type", &str, false );

			if ( stringToLower(str) == "int" )
			{
				(*_generator)->setDBParam( elem->Value(), atoi( elem->GetText().c_str() ));
			}
			else
				(*_generator)->setDBParam( elem->Value(), elem->GetText() );
		}

		node = 0;
		while( node = xml->IterateChildren( "select", node ))
		{
			elem = node->ToElement();

			SelectElements elements;
			elem->GetAttribute( "name", &elements.name );
			getXMLParams( elem, &elements );

			(*_generator)->addSelect( elements );
		}

		node = 0;
		while( node = xml->IterateChildren( "update", node ))
		{
			elem = node->ToElement();

			UpdateElements elements;
			elem->GetAttribute( "name", &elements.name );
			getXMLParams( elem, &elements );

			(*_generator)->addUpdate( elements );
		}

		node = 0;
		while( node = xml->IterateChildren( "insert", node ))
		{
			elem = node->ToElement();

			InsertElements elements;
			elem->GetAttribute( "name", &elements.name );
			getXMLParams( elem, &elements );

			(*_generator)->addInsert( elements );
		}

		node = 0;
		while( node = xml->IterateChildren( "extra", node ))
		{
			subnode = 0;
			while( subnode = node->IterateChildren( "types", subnode ))
			{
				valnode = 0;
				while( valnode = subnode->IterateChildren( valnode ))
				{
					elem = valnode->ToElement();

					String str;
					elem->GetText( &str, false );
					if ( str.empty() )
						elem->GetAttribute( "type", &str, false );

					if ( str.length() )
					{
						SQLTypes type = typeNameToSQLType( elem->Value() );

						if ( type == stUnknown )
						{
							WARNING("unknown type for: " << elem->Value());
						}
						else
							(*_generator)->setType( type, str );
					}
				}
			}

			subnode = 0;
			while( subnode = node->IterateChildren( "namespaces", subnode ))
			{
				valnode = 0;
				while( valnode = subnode->IterateChildren( "namespace", valnode ))
				{
					elem = valnode->ToElement();

					String str;
					elem->GetText( &str, false );
					if ( str.empty() )
						elem->GetAttribute( "name", &str, false );

					if ( !str.empty() )
						(*_generator)->addNamespace( str );
				}
			}

			subnode = 0;
			while( subnode = node->IterateChildren( "headers", subnode ))
			{
				valnode = 0;
				while( valnode = subnode->IterateChildren( "header", valnode ))
				{
					elem = valnode->ToElement();

					String str;
					elem->GetText( &str, false );
					if ( str.empty() )
						elem->GetAttribute( "value", &str, false );

					if ( !str.empty() )
						(*_generator)->addHeader( str );
				}
			}
		}
	}
	catch( ticpp::Exception &e )
	{
		FATAL("XML: " << fileName << ": " << e.what());
	}
}

}
