#include <iostream>
#include <fstream>
#include <stack>

enum SectionType
{
	secOther = 0,
	secOpen,
	secClose
};

struct Section
{
	std::string name;
	int lineNum;
	int charNum;
};

int main(int argc, char **argv)
{
	for(int i = 1; i != argc; i++)
	{
		std::ifstream file(argv[i]);

		std::stack<Section> sections;

		const size_t lineSize = 4098;
		char line[lineSize];
		std::string trailing;

		int lineNum = 0, charNum = 0;

		while(file.good())
		{
			if (!trailing.empty())
			{
				std::size_t len = trailing.copy(line, lineSize);
				trailing.clear();
				file.read(line + len, lineSize - len);
			}
			else
				file.read(line, lineSize);

			const char *c = line, *end = line + file.gcount() - 1;
			const char *sec = 0;
			SectionType type = secOther;

			for(; c != end; c++, charNum++)
			{
				if (*c == '\n')
				{
					lineNum++;
					charNum = 0;
				}
				else if (*c == '{' && *(c + 1) == '{')
				{
					switch (*(c + 2))
					{
						case '#':
							type = secOpen;
							break;
						case '/':
							type = secClose;
							break;
						default:
							type = secOther;
							continue;
					}

					sec = c + 3;
					c += 2;
				}
				else if (sec && *c == '}' && *(c + 1) == '}')
				{
					c += 1;
					std::string name(sec, c - sec - 1);
					sec = 0;

					switch(type)
					{
						case secOpen:
						{
							Section s;
							s.name = name;
							s.lineNum = lineNum;
							s.charNum = charNum;
							sections.push(s);
							break;
						}
						case secClose:
						{
							Section s = sections.top();
							if (s.name.compare(name) != 0)
							{
								std::cerr << "Unmatched section:\n"
											"\tExpected: '" << s.name << "' created at " << s.lineNum << "," << s.charNum << "\n"
											"\tGot: '" << name << "'" << " at " << lineNum << "," << charNum << "\n" <<
											std::flush;
								return 1;
							}
							sections.pop();
							break;
						}
					}
				}
			}

			if (sec)
			{
				trailing = std::string(sec, end - sec);
			}

		}

		if (!sections.empty())
		{
			std::cerr << "Unfinished sections:\n";

			while(!sections.empty())
			{
				Section s = sections.top();
				std::cout << "\t" << s.name << "' created at " << s.lineNum << "," << s.charNum << "\n";
				sections.pop();
			}

			std::cout << std::flush;
		}
	}
	return 0;
}
