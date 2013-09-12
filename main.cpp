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

#include "main.h"
#include "abstractgenerator.h"

#include "sql_reader.h"
#include "xml_reader.h"
#ifdef WITH_YAML
#include "yaml_reader.h"
#endif

#define DEFAULT_TEMLPATE "c++,boost"

#include <boost/program_options.hpp>

namespace DBBinder
{

std::string     appName;
std::string     optOutput;
std::string     optTemplate;
ListString      optTemplateDirs;
const char*     optVersionMajor = "0";
const char*     optVersionMinor = "1";
bool            optListDepends = false;
ListString      optDepends;
bool            optExtras = false;
ListString      optIncludeFiles;

static const char* defaultTemplateDirs[] =
{
    "/usr/share/dbbinder/templates",
    "/usr/local/share/dbbinder/templates",
    "~/share/dbbinder/templates",
    0
};

std::string cescape(const std::string & _string)
{
    std::string result;
    foreach(char c, _string)
    {
        switch( c )
        {
            case '\\':
                result += "\\\\";
                break;
            case '"':
                result += "\\\"";
                break;
            case '\n':
                result += " \\\n";
                break;
            default:
                result += c;
        }
    }
    return result;
}

ListString stringTok(const char* _str)
{
    ListString result;
    const char *c = _str, *start = NULL, *end = _str + strlen(_str);
    
    while(c < end)
    {
        for(;c < end; c++)
        {
            if (!isblank(*c))
            {
                start = c;
                break;
            }
        }
        
        for(;c < end; c++)
        {
            if (isblank(*c))
            {
                result.push_back(std::string(start, c - start));
                start = NULL;
                break;
            }
        }
    }
    
    if (start != NULL)
        result.push_back(std::string(start, c - start));
    
    return result;
}

}

#define FATAL_EXIT(S) do {FATAL(S); return 1;} while (false)

int main(int argc, char *argv[])
{
    using namespace DBBinder;
    namespace po = boost::program_options;

    // Declare the supported options.
    po::options_description desc("Usage");
    desc.add_options()
        ("help,h", "print this help message")
        ("input,i", po::value<std::string>(), "set the input file name")
        ("output,o", po::value<std::string>(), "set the output file name")
        ("xml,x", "set the input format to XML (default)")

#ifdef WITH_YAML
        ("yaml,y", "set the input format to YAML")
#endif
        ("depends,d", "list the dependencies for a target (does nothing else)")
        ("extras,e", "generate any extra files that a template might depend/use")
        ("include,I", po::value<ListString>(), "add an include file")
        ("template-dir,d", po::value<ListString>(), "add a template directory")
        ("template,t", po::value<std::string>()->default_value(DEFAULT_TEMLPATE), "FOO[,BAR] set the template and optional sub-template")
        ("database,db", po::value<std::string>(), "TYPE[,CONN0[,CONN1]] Database to connect and, optionally, connection params\n"
            "\texample: --database MySQL,127.0.0.1,db,user,password"
        )
        ("sql", po::value<std::string>(), "SQL command\n"
            "\texample: --sql 'select FIELD1, FIELD2 from TABLE where FIELD3 = ?'"
        )
        ("sql-param", po::value<ListString>(), "PARAM,TYPE[,DEFAULT] parameters for the SQL command\n"
            "\texample: --param name,string,'no name' --param age,int"
        )
        ("version,V", "program version")
        ("vmajor", "major program version")
        ("vminor", "minor program version");

    appName = extractFileName( argv[0] );

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    std::string fileName;
    FileType fileType = ftNULL;

    if ( argc <= 1 || vm.count("help") )
    {
        std::cout << desc << std::endl;
        return 0;
    }

    if (vm.count("version"))
    {
        std::cout << optVersionMajor << '.' << optVersionMinor << std::endl;
        return 0;
    }

    if (vm.count("vmajor"))
    {
        std::cout << optVersionMajor << std::endl;
        return 0;
    }

    if (vm.count("vminor"))
    {
        std::cout << optVersionMinor << std::endl;
        return 0;
    }

    if (vm.count("input") == 0)
    {
        FATAL_EXIT("missing input file name");
    }
    else
    {
        std::string s = vm["input"].as<std::string>();

        switch (checkFileExistsAndType(s, fctRegularFile))
        {
            case fcOK:                  fileName = s; break;
            case fcDoesNotExist:        FATAL_EXIT( s << ": No such file");
            case fcIsNotExpectedType:   FATAL_EXIT( s << ": must be a regular file");
        }
    }

    if (vm.count("output") == 0)
    {
        optOutput = fileName;
        std::string::size_type pos = optOutput.rfind('.');
        if ( pos == std::string::npos )
        {
            optOutput += "_out";
        }
        else
        {
            optOutput = optOutput.substr(0, pos);
        }
    }
    else
    {
        std::string s = vm["output"].as<std::string>();

        switch (checkFileExistsAndType(s, fctRegularFile))
        {
            case fcOK:
                WARNING(s << " : will be overritten");
            case fcDoesNotExist:
                optOutput = s;
                break;

            case fcIsNotExpectedType:
                FATAL_EXIT( s << ": must be a regular file");
        }
    }

    if (vm.count("xml") + vm.count("yaml") > 1)
    {
        FATAL_EXIT("cannot set more than one file type flag");
    }

    if (vm.count("xml"))
    {
        fileType = ftXML;
    }
    else if (vm.count("yaml"))
    {
#ifdef WITH_YAML
        fileType = ftYAML;
#else
        FATAL_EXIT("no yaml support");
#endif
    }

    optListDepends = vm.count("depends");
    optExtras = vm.count("extras");

    if (vm.count("include"))
        optIncludeFiles = vm["include"].as<ListString>();

    if (vm.count("template"))
        optTemplate = vm["template"].as<std::string>();

    if (vm.count("template-dir"))
    {
        optTemplateDirs = vm["template-dir"].as<ListString>();

        std::string dir;
        foreach(dir, optTemplateDirs)
        {
            switch (checkFileExistsAndType(dir, fctDirectory))
            {
                case fcOK:					break;
                case fcDoesNotExist:		FATAL_EXIT( dir << ": No such directory");
                case fcIsNotExpectedType:	FATAL_EXIT( dir << ": must be a directory");
            }
        }
    }

    assert( !optTemplate.empty() );

    //Add the program's own path
    {
        std::string dir = argv[0];
        dir = dir.substr(0, dir.rfind('/'));
        dir += "/templates";

        if ( checkFileExistsAndType(dir, fctDirectory) == fcOK )
            optTemplateDirs.push_back( dir );
    }

    for( int i = 0; defaultTemplateDirs[i]; i++ )
    {
        // Check to see if default dirs exists before adding them
        if ( checkFileExistsAndType(defaultTemplateDirs[i], fctDirectory) == fcOK )
            optTemplateDirs.push_back( defaultTemplateDirs[i] );
    }

    // If not explicitly selected by the user, deduce the type fromt the file's extension
    if ( fileType == ftNULL )
    {
        std::string::size_type pos = fileName.rfind('.');
        const char *c = fileName.c_str() + pos + 1;

        switch( *c )
        {
            case 's':
                if ( strcasecmp(c, "sql") == 0 )
                {
                    fileType = ftSQL;
                    break;
                }
            case 'x':
                if ( strcasecmp(c, "xml") == 0 )
                {
                    fileType = ftXML;
                    break;
                }
                // no break
            case 'y':
                if ( strcasecmp(c, "yaml") == 0 )
                {
                    fileType = ftYAML;
                    break;
                }
                // no break
            default:
                FATAL(fileName << ": unknown file extension - " << c);
        }
    }

    switch ( fileType )
    {
        case ftXML:
        {
            parseXML(fileName);
            break;
        }
        case ftYAML:
        {
#ifdef WITH_YAML
            parseYAML(fileName);
#else
            FATAL_EXIT("no yaml support");
#endif
            break;
        }
        case ftSQL:
        {
            parseSQL(fileName);
            break;
        }
        default:
            FATAL("Unknown file type.");
    }

    AbstractGenerator* generator = AbstractGenerator::getGenerator();

    // Add the extra include files passed by command-line
    ListString::iterator it = optIncludeFiles.begin(), end = optIncludeFiles.end();
    for(; it != end; ++it)
        generator->addHeader(std::string("#include ") + *it);

    generator->generate();

    if ( optListDepends )
    {
        std::cout << "Depends:\n";

        ListString::iterator it;
        for(it = optDepends.begin(); it != optDepends.end(); ++it)
            std::cout << *it << "\n";

        std::cout << std::flush;
    }
    return 0;
}
