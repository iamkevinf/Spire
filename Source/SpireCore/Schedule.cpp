#include "Schedule.h"
#include "Lexer.h"
using namespace CoreLib::Basic;

namespace Spire
{
	namespace Compiler
	{
		class ScheduleParser
		{
		private:
			List<CompileError>& errors;
			List<Token> tokens;
			int pos;
			String fileName;
			Token & ReadToken(const wchar_t * string)
			{
				if (pos >= tokens.Count())
				{
					errors.Add(CompileError(String(L"\"") + string + String(L"\" expected but end of file encountered."), 0, CodePosition(0, 0, fileName)));
					throw 0;
				}
				else if (tokens[pos].Content != string)
				{
					errors.Add(CompileError(String(L"\"") + string + String(L"\" expected"), 0, tokens[pos].Position));
					throw 20001;
				}
				return tokens[pos++];
			}

			Token & ReadToken(TokenType type)
			{
				if (pos >= tokens.Count())
				{
					errors.Add(CompileError(TokenTypeToString(type) + String(L" expected but end of file encountered."), 0, CodePosition(0, 0, fileName)));
					throw 0;
				}
				else if (tokens[pos].Type != type)
				{
					errors.Add(CompileError(TokenTypeToString(type) + String(L" expected"), 20001, tokens[pos].Position));
					throw 20001;
				}
				return tokens[pos++];
			}

			bool LookAheadToken(const wchar_t * string)
			{
				if (pos >= tokens.Count())
				{
					errors.Add(CompileError(String(L"\'") + string + String(L"\' expected but end of file encountered."), 0, CodePosition(0, 0, fileName)));
					return false;
				}
				else
				{
					if (tokens[pos].Content == string)
						return true;
					else
						return false;
				}
			}
		public:
			ScheduleParser(List<CompileError>& _errorList)
				: errors(_errorList)
			{}
			Schedule Parse(String source, String _fileName)
			{
				this->fileName = _fileName;
				Schedule schedule;
				Lexer lex;
				tokens = lex.Parse(fileName, source, errors);
				pos = 0;
				try
				{
					while (pos < tokens.Count())
					{
						if (LookAheadToken(L"attrib"))
						{
							EnumerableDictionary<String, String> additionalAttributes;
							ReadToken(L"attrib");
							String choiceName = ReadToken(TokenType::Identifier).Content;
							while (LookAheadToken(L"."))
							{
								choiceName = choiceName + L".";
								ReadToken(TokenType::Dot);
								choiceName = choiceName + ReadToken(TokenType::Identifier).Content;
							}
							ReadToken(TokenType::OpAssign);

							while (pos < tokens.Count())
							{
								auto name = ReadToken(TokenType::Identifier).Content;
								String value;
								if (LookAheadToken(L":"))
								{
									ReadToken(L":");
									value = ReadToken(TokenType::StringLiterial).Content;
								}
								additionalAttributes[name] = value;
								if (LookAheadToken(L","))
									ReadToken(TokenType::Comma);
								else
									break;
							}
							schedule.AddtionalAttributes[choiceName] = additionalAttributes;
						}
						else
						{
							String choiceName = ReadToken(TokenType::Identifier).Content;
							while (LookAheadToken(L"."))
							{
								choiceName = choiceName + L".";
								ReadToken(TokenType::Dot);
								choiceName = choiceName + ReadToken(TokenType::Identifier).Content;
							}
							ReadToken(TokenType::OpAssign);
							List<RefPtr<ChoiceValueSyntaxNode>> worlds;
							while (pos < tokens.Count())
							{
								auto & token = ReadToken(TokenType::StringLiterial);
								RefPtr<ChoiceValueSyntaxNode> choiceValue = new ChoiceValueSyntaxNode();
								choiceValue->Position = token.Position;
								int splitterPos = token.Content.IndexOf(L':');
								if (splitterPos != -1)
								{
									choiceValue->WorldName = token.Content.SubString(0, splitterPos);
									choiceValue->AlternateName = token.Content.SubString(splitterPos + 1, token.Content.Length() - splitterPos - 1);
								}
								else
								{
									choiceValue->WorldName = token.Content;
								}
								worlds.Add(choiceValue);
								if (LookAheadToken(L","))
									ReadToken(TokenType::Comma);
								else
									break;
							}
							schedule.Choices[choiceName] = worlds;
						}
						ReadToken(TokenType::Semicolon);
					}
				}
				catch (...)
				{
				}
				return schedule;
			}
		};
	
		Schedule Schedule::Parse(String source, String fileName, List<CompileError>& errorList)
		{
			return ScheduleParser(errorList).Parse(source, fileName);
		}
	}
}