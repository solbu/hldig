//
// WordRecord.cc
//
// WordRecord: data portion of the inverted index database
//
#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif /* HAVE_CONFIG_H */

#include <iostream.h>
#include <stdlib.h>

#include "WordRecord.h"

ostream &operator << (ostream &o, const WordRecord &record)
{
  switch(record.type) {

  case WORD_RECORD_DATA:
    o << record.info.data;
    break;

  case WORD_RECORD_STATS:
    o << record.info.stats.noccurence << "\t";
    o << record.info.stats.ndoc;
    break;

  case WORD_RECORD_NONE:
    break;

  default:
    cerr << "WordRecord::ostream <<: unknown type " << record.type << "\n";
    break;
  }

  return o;
}

//
// Set a record from an ascii representation
//
int
WordRecord::Set(const String& buffer)
{
  StringList fields(buffer, "\t ");
  return Set(fields);
}

int
WordRecord::Set(StringList& fields)
{
  int i = 0;
  
  switch(type) 
    {
	
    case WORD_RECORD_DATA:
      {
	String* field = (String*)fields.Get_First();

	if(field == 0) {
	  cerr << "WordRecord::Set: failed to retrieve field " << i << endl;
	  return NOTOK;
	}
	info.data = (unsigned int)atoi(field->get());
	fields.Remove(field);
	i++;
      }
      break;

    case WORD_RECORD_STATS:
      {
	String* field = (String*)fields.Get_First();

	if(field == 0) {
	  cerr << "WordRecord::Set: failed to retrieve field " << i << endl;
	  return NOTOK;
	}
	info.stats.noccurence = (unsigned int)atoi(field->get());
	fields.Remove(field);
	i++;

	field = (String*)fields.Get_First();

	if(field == 0) {
	  cerr << "WordRecord::Set: failed to retrieve field " << i << endl;
	  return NOTOK;
	}
	info.stats.ndoc = (unsigned int)atoi(field->get());
	fields.Remove(field);
	i++;
      }
      break;

    case WORD_RECORD_NONE:
      break;

    default:
      cerr << "WordRecord::ostream <<: unknown type " << type << "\n";
      break;
    }

  return OK;
}

void WordRecord::Print() const
{
  cout << *this;
}
