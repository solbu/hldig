//
// ResultMatch.cc
//
// Implementation of ResultMatch
//
// $Id: ResultMatch.cc,v 1.3 1999/04/19 01:21:51 hp Exp $
//

#include "ResultMatch.h"

// Definition of how to search
ResultMatch::SortType ResultMatch::mySortType;

//*****************************************************************************
//
ResultMatch::ResultMatch()
{
}


//*****************************************************************************
//
ResultMatch::~ResultMatch()
{
}


//*****************************************************************************
// Default-access-methods.  Just dummies when that data is not used.
char *ResultMatch::getTitle()
{ return ""; }

time_t ResultMatch::getTime()
{ return 0; }

void ResultMatch::setTitle(char *)
{ }

void ResultMatch::setTime(time_t)
{ }

// Then for each sort-type, we derive a class, which will keep
// any necessary additional piece of data, and return the compare-function.

// We could have a real cute implementation with global
// constructors registering a factory method with ResultMatch,
// so it would just check a list and never need to be changed
// when new search methods are introduced, but that seems futile.
//  It is more practical to just add search methods here and
// change the createMatch method, last.


//*****************************************************************************
class ScoreMatch : public ResultMatch
{
  // This one needs no additional data
public:
  virtual ResultMatch::CmpFun getSortFun();
  ScoreMatch();
  ~ScoreMatch();
private:
  static int compare(const void *a1, const void *a2);
};

ScoreMatch::ScoreMatch() {}
ScoreMatch::~ScoreMatch() {}

int
ScoreMatch::compare(const void *a1, const void *a2)
{
    ResultMatch	*m1 = *((ResultMatch **) a1);
    ResultMatch	*m2 = *((ResultMatch **) a2);

    return m2->getScore() - m1->getScore();
}

ResultMatch::CmpFun
ScoreMatch::getSortFun() { return compare; }

//*****************************************************************************
class TimeMatch : public ResultMatch
{
public:
  virtual ResultMatch::CmpFun getSortFun();
  virtual void setTime(time_t);
  virtual time_t getTime();
  TimeMatch();
  ~TimeMatch();
private:
  // We need a time_t here, and to override the get/setTime methods.
  time_t myTime;

  static int compare(const void *a1, const void *a2);
};

TimeMatch::TimeMatch() {}
TimeMatch::~TimeMatch() {}

void
TimeMatch::setTime(time_t t)
{
  myTime = t;
}

time_t TimeMatch::getTime()
{
  return myTime;
}

int
TimeMatch::compare(const void *a1, const void *a2)
{
    ResultMatch	*m1 = *((ResultMatch **) a1);
    ResultMatch	*m2 = *((ResultMatch **) a2);
    time_t	t1 = m1->getTime();
    time_t	t2 = m2->getTime();

    return (int) (t2 - t1);
}

ResultMatch::CmpFun
TimeMatch::getSortFun() { return compare; }

//*****************************************************************************
class IDMatch : public ResultMatch
{
  // This one needs no additional data
public:
  virtual ResultMatch::CmpFun getSortFun();
  IDMatch();
  ~IDMatch();
private:
  static int compare(const void *a1, const void *a2);
};

IDMatch::IDMatch() {}
IDMatch::~IDMatch() {}

int
IDMatch::compare(const void *a1, const void *a2)
{
    ResultMatch	*m1 = *((ResultMatch **) a1);
    ResultMatch	*m2 = *((ResultMatch **) a2);
    int               i1 = m1->getID();
    int               i2 = m2->getID();

    return (i1 - i2);
}

ResultMatch::CmpFun
IDMatch::getSortFun() { return compare; }

//*****************************************************************************
class TitleMatch : public ResultMatch
{
public:
  virtual ResultMatch::CmpFun getSortFun();
  virtual void setTitle(char *t);
  virtual char *getTitle();
  TitleMatch();
  ~TitleMatch();
private:
  // We need a String here, and to override the get/setTitle methods.
  // It has to be a String, as the "char *" goes away shortly
  // after creating the object.
  String myTitle;

  static int compare(const void *a1, const void *a2);
};

TitleMatch::TitleMatch() {}
TitleMatch::~TitleMatch() {}

void
TitleMatch::setTitle(char *t)
{
  myTitle = t;
}

char *
TitleMatch::getTitle()
{
  return myTitle;
}

int
TitleMatch::compare(const void *a1, const void *a2)
{
    ResultMatch	*m1 = *((ResultMatch **) a1);
    ResultMatch	*m2 = *((ResultMatch **) a2);
    char	*t1 = m1->getTitle();
    char	*t2 = m2->getTitle();

    if (!t1) t1 = "";
    if (!t2) t2 = "";
    return mystrcasecmp(t1, t2);
}

ResultMatch::CmpFun
TitleMatch::getSortFun() { return compare; }

//*****************************************************************************
int
ResultMatch::setSortType(char *sorttype)
{
    static const struct
    {
	char		*typest;
	SortType	type;
    }
    sorttypes[] =
    {
	{"score", SortByScore},
	{"date", SortByTime},
	{"time", SortByTime},
        {"title", SortByTitle},
        {"id", SortByID}
    };
    int		i = 0;
    char	*st = sorttype;
    if (st && *st)
    {
	if (mystrncasecmp("rev", st, 3) == 0)
	    st += 3;
	for (i = sizeof(sorttypes)/sizeof(sorttypes[0]); --i >= 0; )
	{
	    if (mystrcasecmp(sorttypes[i].typest, st) == 0)
	    {
		mySortType = sorttypes[i].type;
		return 1;
	    }
	}
	return 0;
    }
    else
    {
      // If not specified, default to SortByScore
      mySortType = SortByScore;
      return 1;
    }
}

//*****************************************************************************
// Now here's the switchboard: a create-function that returns a
// "new":ed object of the right class for what to compare.
//  To have the pairing managed in a (dynamically registered)
// list may seem interesting, but since everything is here
// anyway, there's little need but a small cuteness-factor.
//  We could also change the guts to use some kind of creator
// object, if there would be a win.

ResultMatch *
ResultMatch::create()
{
  switch (mySortType)
  {
    case ResultMatch::SortByScore:
      return new ScoreMatch();

    case ResultMatch::SortByTime:
      return new TimeMatch();

    case ResultMatch::SortByTitle:
      return new TitleMatch();

    case ResultMatch::SortByID:
      return new IDMatch();

    default:
      // It is doubtful which is better: to abort() or paper
      // over something bad here.
      return new ScoreMatch();
  }
}
