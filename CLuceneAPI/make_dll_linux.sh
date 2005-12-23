#!/bin/sh

set -x

export VERSION='0.8.13'
export CLpath ='../../CLucene'
export CLlibpath=$CLpath'/build/gcc/src/.libs'
export GCCpath='/nfs/local/linux/gcc-3.2.2'
rm -rf .libs/
mkdir .libs

g++ -DHAVE_CONFIG_H -I. -I$CLpath/build/gcc -I$CLpath/src -g -ggdb  -O2 -c -o .libs/CLucene_API.o CLucene_API.cpp


g++ -shared -nostdlib /usr/lib/crti.o $GCCpath/lib/gcc-lib/i686-pc-linux-gnu/3.2.2/crtbeginS.o $CLlibpath/Analyzers.o $CLlibpath/StandardAnalyzer.o $CLlibpath/StandardFilter.o $CLlibpath/StandardTokenizer.o $CLlibpath/condition.o $CLlibpath/DateField.o $CLlibpath/Document.o $CLlibpath/Field.o $CLlibpath/DocumentWriter.o $CLlibpath/SegmentMergeInfo.o $CLlibpath/SegmentsReader.o $CLlibpath/FieldInfos.o $CLlibpath/SegmentMergeQueue.o $CLlibpath/Term.o $CLlibpath/FieldsReader.o $CLlibpath/SegmentMerger.o $CLlibpath/TermInfo.o $CLlibpath/FieldsWriter.o $CLlibpath/SegmentReader.o $CLlibpath/TermInfosReader.o $CLlibpath/IndexReader.o $CLlibpath/SegmentTermDocs.o $CLlibpath/TermInfosWriter.o $CLlibpath/IndexWriter.o $CLlibpath/SegmentTermEnum.o $CLlibpath/SegmentInfos.o $CLlibpath/SegmentTermPositions.o $CLlibpath/Lexer.o $CLlibpath/QueryParser.o $CLlibpath/QueryParserBase.o $CLlibpath/QueryToken.o $CLlibpath/TokenList.o $CLlibpath/MultiFieldQueryParser.o $CLlibpath/BooleanQuery.o $CLlibpath/IndexSearcher.o $CLlibpath/Similarity.o $CLlibpath/BooleanScorer.o $CLlibpath/MultiTermQuery.o $CLlibpath/SloppyPhraseScorer.o $CLlibpath/DateFilter.o $CLlibpath/PhrasePositions.o $CLlibpath/TermQuery.o $CLlibpath/ExactPhraseScorer.o $CLlibpath/PhraseQuery.o $CLlibpath/TermScorer.o $CLlibpath/FilteredTermEnum.o $CLlibpath/PhraseScorer.o $CLlibpath/TopDocs.o $CLlibpath/HitQueue.o $CLlibpath/PrefixQuery.o $CLlibpath/WildcardQuery.o $CLlibpath/Hits.o $CLlibpath/RangeQuery.o $CLlibpath/WildcardTermEnum.o $CLlibpath/FuzzyQuery.o $CLlibpath/MultiSearcher.o $CLlibpath/FSDirectory.o $CLlibpath/InputStream.o $CLlibpath/Lock.o $CLlibpath/OutputStream.o $CLlibpath/RAMDirectory.o $CLlibpath/Arrays.o $CLlibpath/FastCharStream.o $CLlibpath/Reader.o $CLlibpath/BitVector.o $CLlibpath/Misc.o $CLlibpath/StringBuffer.o $CLlibpath/LuceneUTF8.o $CLlibpath/StdHeader.o .libs/CLucene_API.o -Wl,--whole-archive $GCCpath/lib/./libstdc++.a -Wl,--no-whole-archive -L/nfs/local/src/gcc/gcc-3.2.2/gcc -L/nfs/local/src/gcc/gcc-3.2.2/i686-pc-linux-gnu/libstdc++-v3/src/.libs -L/nfs/local/src/gcc/gcc-3.2.2/i686-pc-linux-gnu/libstdc++-v3/src -L$GCCpath/lib/gcc-lib/i686-pc-linux-gnu/3.2.2 -L$GCCpath/lib/gcc-lib/i686-pc-linux-gnu/3.2.2/../../.. -lm -lc $GCCpath/lib/gcc-lib/i686-pc-linux-gnu/3.2.2/crtendS.o /usr/lib/crtn.o -Wl,-soname -Wl,libcluceneapi.so.$VERSION -o .libs/libcluceneapi.so.$VERSION



ls  -altr .libs/
ldd .libs/libcluceneapi.so.$VERSION

cp .libs/libcluceneapi.so.$VERSION .

