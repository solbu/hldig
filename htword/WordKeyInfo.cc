// WordKeyInfo.cc
//
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
//

#include "WordKeyInfo.h"
#include "WordKey.h"
#include <stdlib.h>
#include<iostream.h>
#include<fstream.h>


void 
WordKeyField::nprint(char c,int n)
{
    for(int i=0;i<n;i++)
    {
	if(!(i%4)){printf("%c",'a'+i/4);}
	else{printf("%c",c);}
    }
}

void 
WordKeyInfo::SetKeyDescriptionFromString(const String &desc)
{
    if(WordContext::key_info){delete WordContext::key_info;}
    WordContext::key_info=new WordKeyInfo();
    WordContext::key_info->SetDescriptionFromString(desc);
}
void 
WordKeyInfo::SetKeyDescriptionFromFile(const String &filename)
{
    if(WordContext::key_info){delete WordContext::key_info;}
    WordContext::key_info=new WordKeyInfo();
    WordContext::key_info->SetDescriptionFromFile(filename);
}

void 
WordKeyField::show()
{
    nprint(' ',bits_offset);
    printf("\"%s\" %3d %3d type:%2d lowbits:%2d lastbits:%2d\n",(char *)name,encoding_position,sort_position,type, lowbits, lastbits);
    nprint(' ',bits_offset);
    printf("|---bytesize:%2d bytes_offset:%2d bits:%2d direction:%2d\n", bytesize, bytes_offset, bits, direction);
    nprint(' ',bits_offset);
    printf("|---encoding_position:%2d sort_position:%2d bits_offset:%2d\n", encoding_position, sort_position, bits_offset);

}

WordKeyField::WordKeyField(WordKeyField *previous,char *nname,int nbits,int nencoding_position, int nsort_position )
{
    encoding_position=nencoding_position;
    sort_position=nsort_position;
    name = strdup(nname);

    type=(sort_position ? WORD_ISA_NUMBER : WORD_ISA_String);
    bits = nbits;
    bits_offset = (previous ?  previous->bits_offset + previous->bits  :  0 );

    if(bits_offset%8 && sort_position==0)
    {
	cerr << "WordKeyField::WordKeyField: begining of word is at:" << bits_offset
	     << " should be byte aligned" << endl;	
	bits_offset+=8-(bits_offset%8);
    }

    if(bits_offset<0 || bits_offset>WORDKEYFIELD_BITS_MAX*WORD_KEY_MAX_NFIELDS)
    {
	cerr << "WordKeyField::WordKeyField: bits_offset:" << bits_offset << " is not correct" << endl;
    }
    bytes_offset = bits_offset/8;
    bytesize = (bits_offset+bits-1)/8 - bytes_offset + 1;
    lastbits = (bits_offset+bits)%8;
    lowbits  =  bits_offset%8;
    direction=WORD_SORT_ASCENDING;
}
void 
WordKeyInfo::Initialize( String &line)
{
    StringList fields(line, "\t ");

    String *found;
    fields.Start_Get();
    int i=0;
    int nnfields=-1;
    while((found = (String*)fields.Get_Next()))
    {
	if(i==1)
	{
	    nnfields=atoi((char *)*found);
	}
	else
	{
	    if(i>1 || (*found!=String("nfields:") && *found!=String("nfields")) )
	    {
		cerr << "WordKeyInfo::Initialize: syntax error at begining entry:" << i <<
		    " line:\"" << line << "\"" << endl;
	    }
	}
	i++;
    }
    if(nnfields<2 || nnfields>=WORD_KEY_MAX_NFIELDS)
    {
	cerr << "WordKeyInfo::Initialize: invalid nfields:" << nnfields << endl;
    }
    Initialize(nnfields);
}
void 
WordKeyInfo::Initialize( int nnfields)
{
//      cerr << "WordKeyInfo::Initialize: nfields:" << nnfields << endl;
    nfields = nnfields;
    sort = new WordKeyField[nfields];
    encode = new WordKeyField[nfields];
    minimum_length = 0;
    previous=NULL;
    encoding_position=0;
}


void 
WordKeyInfo::AddFieldInEncodingOrder(String &name,int bits, int sort_position)
{
    int i;
    for(i=0;i<encoding_position;i++)
    {
	if(encode[i].sort_position==sort_position)
	{
	    cerr << "WordKeyInfo::AddFieldInEncodingOrder: syntax error in key: " << endl;
	    cerr << "WordKeyInfo::AddFieldInEncodingOrder: found sort position twice " << endl;
	}
    }
	

    WordKeyField tmp( previous, name, bits, encoding_position, sort_position );
    sort[sort_position]       = tmp;
    encode[encoding_position] = tmp;
//    printf ("srt:");sort[sort_position].show();
    previous = sort + sort_position;

    encoding_position++;
    if(sort_position == 0)
    {
	// this should be the last encoded field...
	minimum_length = sort[sort_position].bytes_offset;
	// verifiy some things
	int fail=0;
	if(encoding_position!=nfields)
	{
	    cerr << "WordKeyInfo::AddFieldInEncodingOrder: didnt find the right nuimber of fields" << endl;
	    cerr << "found:" << encoding_position << " expected:" << nfields << endl;
	}
  	for(i=0;i<nfields;i++)
  	{
  	    if(sort[i].sort_position!=i){fail=1;}
  	    if(encode[sort[i].encoding_position].sort_position!=i){fail=2;}

  	    if(sort[encode[i].sort_position].encoding_position!=i){fail=3;}
  	    if(encode[i].encoding_position!=i){fail=4;}
  	}
	if(fail)
	{
	    cerr << "WordKeyInfo::AddFieldInEncodingOrder: bad syntax:" << fail << endl;
	    for(i=0;i<nfields;i++)
	    {
		cerr << "field in encoding order:" << i << endl;
		encode[i].show();
	    }
	    for(i=0;i<nfields;i++)
	    {
		cerr << "field in sort order:" << i << endl;
		sort[i].show();
	    }
	}
    }
}

void 
WordKeyInfo::AddFieldInEncodingOrder(const String &line)
{
//      cerr << "AddFieldInEncodingOrder::  :" << line << endl;
    StringList fields(line, "\t ");
	
    fields.Start_Get();

    String *name=(String*)fields.Get_Next();
    if(!name){cerr << "WordKeyInfo::AddFieldInEncodingOrder: bad syntax:" << line << endl;}

    String *sbits=(String*)fields.Get_Next();
    if(!sbits){cerr << "WordKeyInfo::AddFieldInEncodingOrder: bad syntax:" << line << endl;}
    int bits=atoi((char *)*sbits);
    if(bits<0 || bits>WORDKEYFIELD_BITS_MAX){cerr << "WordKeyInfo::AddFieldInEncodingOrder: strange value:" << line << endl;}

    String *ssortpos=(String*)fields.Get_Next();
    if(!ssortpos){cerr << "WordKeyInfo::AddFieldInEncodingOrder: bad syntax:" << line << endl;}
    int sortpos=atoi((char *)*ssortpos);
    if(sortpos<0 || sortpos>nfields){cerr << "WordKeyInfo::AddFieldInEncodingOrder: strange value:" << line << endl;}

    AddFieldInEncodingOrder(*name, bits, sortpos);
}

void
WordKeyInfo::SetDescriptionFromString(const String &desc)
{
//      cerr << "WordKeyInfo::SetKeyDescriptionFromString:\"" << desc << "\""<< endl;
    StringList lines(desc, "/");
    String *found;
    int initok=0;
    lines.Start_Get();
    while((found = (String*)lines.Get_Next()))
    {
	if(!initok){Initialize(*found);initok=1;}
	else
	{
	    if( encoding_position >= nfields )
	    {cerr << "WordKeyInfo::SetKeyDescriptionFromString: unexpected line " << *found << endl;}
	    AddFieldInEncodingOrder(*found);
	}
    }
}
void
WordKeyInfo::SetDescriptionFromFile(const String &filename)
{
     cerr << "WordKeyInfo::SetKeyDescriptionFromFile:" << filename << endl;
    nfields=-1;
    ifstream in((const char *)filename);
#define WORD_BUFFER_SIZE	1024
    char buffer[WORD_BUFFER_SIZE];
    String line;
    int line_number = 0;

    while(!in.eof())
    {
	line_number++;

	in.get(buffer, WORD_BUFFER_SIZE);
	line.append(buffer);
	//
	// Get the terminator. I love iostream :-(
	//
	if(!in.eof()) 
	{
	    char c;
	    in.get(c);
	    if(c == '\n') 
		line.append(c);
	    in.putback(c);
	}
	//
	// Join big lines
	//
	if(line.last() != '\n' && line.last() != '\r' && !in.eof())
	    continue;
	//
	// Eat the terminator
	//
	if(!in.eof()) in.get();
	//
	// Strip line terminators from line
	//
	line.chop("\r\n");
	//
	// If line ends with a \ continue
	//
	if(line.last() == '\\') {
	    line.chop(1);
	    if(!in.eof())
		continue;
	}
      
	if(!line.empty()) 
	{
	    if(!in.good())
	    {
		cerr << "WordKeyInfo::SetKeyDescription: line " << line_number << " : " << line << endl
		     << " input from file failed (A)" << endl;
		break;
	    }

	    if(line[0] != '#')
	    {

		if( nfields < 0 ){Initialize(line);}
		else
		{
		    AddFieldInEncodingOrder(line);
		}

//  		if( encoding_position >= nfields )
//  		{cerr << "WordKeyInfo::SetKeyDescription: unexpected line " << line_number << " : " << line << endl;}
	    }
      
	    if(in.eof()){break;}
	    if(!in.good())
	    {
		cerr << "WordKeyInfo::SetKeyDescription: line " << line_number << " : " << line << endl
		     << " input from stream failed (B)" << endl;
		break;
	    }
	}

	line.trunc();
    }
}

void 
WordKeyInfo::show()
{
    printf("-----------------------------------------\n");
    printf("nfields:%3d minimum_length:%3d\n",nfields,minimum_length);
    int i,j;
    for(i=0;i<nfields;i++)
    {
	for(j=0;j<nfields;j++){if(sort[j].encoding_position==i)break;}
	if(j==nfields)
	{
	    cerr << "WordKeyInfo::show field not found !!!!!!!!!!!!!! " <<endl;
	    j=0;
	}
	sort[j].show();
    }
    char str[WORDKEYFIELD_BITS_MAX*WORD_KEY_MAX_NFIELDS];
    for(i=0;i<WORDKEYFIELD_BITS_MAX*WORD_KEY_MAX_NFIELDS;i++)
    {str[i]='_';}
    int tmx=0;
    for(j=0;j<nfields;j++)
    {
	for(i=0;i<sort[j].bits;i++)
	{
	    char c=(j%10)+'0';
	    int pos=sort[j].bits_offset+i;
	    if(str[pos]!='_'){c='X';}
	    str[pos]=c;
	    if(tmx<pos){tmx=pos;}
	}
    }
    str[tmx+1]=0;
    printf("%s (bits)\n",str);
    printf("^0      ^1      ^2      ^3      ^4      ^5      ^6      ^7\n");
    printf("0123456701234567012345670123456701234567012345670123456701234567\n");
}

void 
WordKeyInfo::Initialize(const Configuration &config)
{
    const String &keydescfile = config["wordlist_wordkey_description_file"];
    const String &keydesc     = config["wordlist_wordkey_description"];

    if(!keydesc.empty())
    {
	WordKeyInfo::SetKeyDescriptionFromString(keydesc);
    }
    else
    if(!keydescfile.empty())
    {
	WordKeyInfo::SetKeyDescriptionFromFile(keydescfile);
    }
    else
    {
	cerr << "WordList::Initialize: didn't find key description file in config" << endl;
    }

}
