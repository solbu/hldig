#!/bin/sh

set -x

export VERSION='0.8.13'
export CLpath='../../CLucene'
export CLlibpath=$CLpath'/build/gcc/src/.libs'



#
# this is the ugly part. you need to correct these two blocks of includes
# to bring them in line with your system's particular setup. we will eventually
# turn this into a some kind of real makefile, but for right now, it's all we've
# got... how you can find this out is to configure/build CLucene (which you've
# done already), and steal its entire library link lines that configure wrote into
# the makefiles, and put the CLucene_API.o in there. Then you can just name the library
# whatever you want and you're good to go (this name will be used later in htdig).
#
export GCClink='/usr/lib/crti.o /usr/lib/gcc/i386-redhat-linux/3.4.3/crtbeginS.o'

export GCClink2='-Wl,--whole-archive /usr/lib/gcc/i386-redhat-linux/3.4.3/libstdc++.a -Wl,--no-whole-archive -L/usr/lib/gcc/i386-redhat-linux/3.4.3/ -lm -lc /usr/lib/gcc/i386-redhat-linux/3.4.3/crtendS.o /usr/lib/crtn.o'

rm -rf .libs/
mkdir .libs

g++ -DHAVE_CONFIG_H -I. -I$CLpath/build/gcc -I$CLpath/src -g -ggdb  -O2 -c -o .libs/CLucene_API.o CLucene_API.cpp


g++ -shared -nostdlib $GCClink $CLlibpath/Analyzers.o $CLlibpath/StandardAnalyzer.o $CLlibpath/StandardFilter.o $CLlibpath/StandardTokenizer.o $CLlibpath/condition.o $CLlibpath/DateField.o $CLlibpath/Document.o $CLlibpath/Field.o $CLlibpath/DocumentWriter.o $CLlibpath/SegmentMergeInfo.o $CLlibpath/SegmentsReader.o $CLlibpath/FieldInfos.o $CLlibpath/SegmentMergeQueue.o $CLlibpath/Term.o $CLlibpath/FieldsReader.o $CLlibpath/SegmentMerger.o $CLlibpath/TermInfo.o $CLlibpath/FieldsWriter.o $CLlibpath/SegmentReader.o $CLlibpath/TermInfosReader.o $CLlibpath/IndexReader.o $CLlibpath/SegmentTermDocs.o $CLlibpath/TermInfosWriter.o $CLlibpath/IndexWriter.o $CLlibpath/SegmentTermEnum.o $CLlibpath/SegmentInfos.o $CLlibpath/SegmentTermPositions.o $CLlibpath/Lexer.o $CLlibpath/QueryParser.o $CLlibpath/QueryParserBase.o $CLlibpath/QueryToken.o $CLlibpath/TokenList.o $CLlibpath/MultiFieldQueryParser.o $CLlibpath/BooleanQuery.o $CLlibpath/IndexSearcher.o $CLlibpath/Similarity.o $CLlibpath/BooleanScorer.o $CLlibpath/MultiTermQuery.o $CLlibpath/SloppyPhraseScorer.o $CLlibpath/DateFilter.o $CLlibpath/PhrasePositions.o $CLlibpath/TermQuery.o $CLlibpath/ExactPhraseScorer.o $CLlibpath/PhraseQuery.o $CLlibpath/TermScorer.o $CLlibpath/FilteredTermEnum.o $CLlibpath/PhraseScorer.o $CLlibpath/TopDocs.o $CLlibpath/HitQueue.o $CLlibpath/PrefixQuery.o $CLlibpath/WildcardQuery.o $CLlibpath/Hits.o $CLlibpath/RangeQuery.o $CLlibpath/WildcardTermEnum.o $CLlibpath/FuzzyQuery.o $CLlibpath/MultiSearcher.o $CLlibpath/FSDirectory.o $CLlibpath/InputStream.o $CLlibpath/Lock.o $CLlibpath/OutputStream.o $CLlibpath/RAMDirectory.o $CLlibpath/Arrays.o $CLlibpath/FastCharStream.o $CLlibpath/Reader.o $CLlibpath/BitVector.o $CLlibpath/Misc.o $CLlibpath/StringBuffer.o $CLlibpath/LuceneUTF8.o $CLlibpath/StdHeader.o .libs/CLucene_API.o $GCClink2 -Wl,-soname -Wl,libcluceneapi.so.$VERSION -o .libs/libcluceneapi.so.$VERSION



ls  -altr .libs/
ldd .libs/libcluceneapi.so.$VERSION

cp .libs/libcluceneapi.so.$VERSION .

