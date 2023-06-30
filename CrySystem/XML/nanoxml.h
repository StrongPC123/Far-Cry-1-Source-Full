// one of the tiniest but still very functional xml parsers (see test.cpp for example usage)
// by Wouter van Oortmerssen, ZLIB license applies.
#ifndef NANO_XML
#define NANO_XML

struct ParseError { char *err; ParseError(char *e) : err(e) {}; };

struct NanoXMLSink
{
	virtual void StartDocument(bool unicode)=0;
	virtual void StartElement(wchar_t *name)=0;
	virtual void EndElement()=0;
	virtual void Data(wchar_t *name)=0;
	virtual void Attribute(wchar_t *name,wchar_t *value)=0;
	virtual void EndDocument(const char *error)=0;
	virtual unsigned int GetByte()=0;
};

struct NanoXML
{
	int token, ch, lineno, linepos;
	bool unicode;
	NanoXMLSink *sink;

	void Parse(NanoXMLSink *_sink)
	{
		sink = _sink;
		lineno = linepos = 1;
		unicode = false;
		GetC();
		if(ch==0xFF)
		{
			GetC();
			if(ch==0xFE)
			{
				unicode = true;
				GetC();
				linepos = 1;
			};
		};
		sink->StartDocument(unicode);
		try
		{
			Lex();
			Expect('<');
			ParseNode();
		}
		catch(ParseError pe)
		{
			sink->EndDocument(pe.err);
			return;
		};
		sink->EndDocument(NULL);	// meaning no error
		return;
	};

	void Expect(int n)
	{
		if(token!=n) Error("unexpected token");
		Lex();
	};

	void Error(char *e) { throw ParseError(e); };

	/* void AddList(NanoNode **&n, NanoNode *c)
	{
	*n = Node(c, false);
	n = &(*n)->next;
	};*/

	int GetC()
	{
		ch = sink->GetByte();
		if(unicode) ch |= sink->GetByte()<<8;
		linepos++;
		return ch;
	};

	void ExpectWord() { if(token!='\"') Error("word expected"); };
	void NewLine() { lineno++; linepos = 0; };

#define MAXWORDSIZE 10000
#define AddToWord(c) { if(p-word>=MAXWORDSIZE-1) Error("increase max word size"); *p++ = c; }
	wchar_t word[MAXWORDSIZE], attrib[MAXWORDSIZE];

	void Lex()
	{
		for(wchar_t *p = word;;) switch(token = ch)
		{
			case '\n':
				NewLine();
			case ' ': case '\t': case '\r':
				GetC();
				continue;

			case 0:
			case -1:
				return;
			
			case '=':
			case '>':
				GetC();
				return;

			case '<':
				GetC();
				if(ch=='!')
				{
					GetC();
					if(ch=='[')
					{
						if(GetC()!='C' || GetC()!='D' || GetC()!='A' || GetC()!='T' || GetC()!='A' || GetC()!='[') Error("CDATA block expected");
						GetC();
						if(ch=='\r') { GetC(); if(ch=='\n') GetC(); };
						for(;;)
						{
							if(ch<=0) Error("EOF inside CDATA block");
							if(ch=='\n') NewLine();
							AddToWord(ch);
							GetC();
							if(p-word>=3 && p[-1]=='>' && p[-2]==']' && p[-3]==']') break;
						};
						p -= 3;
						*p++ = 0;
						token = '\"';
						return;
					}
					else
					{
						int nest = 1;
						while(nest) { if(ch=='>') nest--; if(ch=='<') nest++; GetC(); };
						continue;
					};
				};
				return;

			case '/':
				GetC();
				if(ch=='>') { GetC(); token = '/>'; };
				return;

			case '\"':
				GetC();
				while(ch!='\"' && ch!='\n' && ch>=0) { AddToWord(ch); GetC(); };
				GetC();
				*p++ = 0;
				return;

			default:
				while(ch>' ' && ch!='<' && ch!='>' && ch!='/' && ch!='=')
				{
					AddToWord(ch);
					GetC();
				};
				if(p-word==0) Error("empty word");
				*p++ = 0;
				token = '\"';
				return;
		};
	};

	void ParseNode()
	{
		// Skip <? nodes.
		for(;;)
		{
			ExpectWord();
			if(word[0]=='?')	// assume xml header, just skip to ?>
			{
				for(;;)
				{
					Lex();
					if(token=='\"' && word[0]=='?') break;
				};
				Lex();
				Expect('>');
				Expect('<');
			}
			else
				break;
		};

		ExpectWord();
		sink->StartElement(word);
		wchar_t elemname[256];
		wcscpy(elemname,word);
		Lex();

		for(;;)
		{
			if(token=='>')  { Lex(); break; };
			if(token=='/>') { Lex(); sink->EndElement(); return; }; 
			ExpectWord();
			if(word[0]=='?')	// assume xml header, just skip to ?>
			{
				/*
				for(;;)
				{
					Lex();
					if(token=='\"' && word[0]=='?') break;
				};
				*/
				Lex();
				Expect('>');
				break;
			}
			else
			{
				wcscpy(attrib,word);
				Lex();
				Expect('=');
				ExpectWord();
				sink->Attribute(attrib, word);
				Lex();
			}
		};

		for(;;)
		{
			if(token=='\"') { sink->Data(word); Lex(); continue; };
			Expect('<');
			if(token!='/') { ParseNode(); continue; };
			Lex();
			ExpectWord();  
			if(wcscmp(word, elemname)) Error("closing tag and opening don't match");
			sink->EndElement();
			Lex();
			Expect('>');
			return;
		};
	};
};
#endif //NANO_XML