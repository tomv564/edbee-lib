/**
 * Copyright 2011-2012 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#include "textrangetest.h"

#include <QStringList>
#include <QDebug>

#include "edbee/models/textbuffer.h"
#include "edbee/models/chardocument/chartextdocument.h"
#include "edbee/models/textrange.h"
#include "edbee/views/textselection.h"
#include "edbee/texteditorcontroller.h"

#include "debug.h"


namespace edbee {


#define testRanges( expected ) \
do { \
    QString ranges = selRef_->rangesAsString(); \
    testEqual( ranges, expected ); \
} while(false)



/// Creates a selection object
/// @param doc the document to use
/// @param definition the definition. In the format  anchor>caret,anchor>caret
static void addRanges( TextRangeSet* sel, const QString& definition )
{
    QStringList ranges = definition.split(",", QString::SkipEmptyParts);
    foreach( QString range, ranges ) {
        QStringList values = range.split(">");
        Q_ASSERT(values.length() == 2);
        int anchor = values.at(0).trimmed().toInt();
        int caret  = values.at(1).trimmed().toInt();
        sel->addRange(caret,anchor);
    }
}



void TextRangeTest::init()
{
    doc_ = new CharTextDocument();
    bufRef_ = doc_->buffer();
}

void TextRangeTest::clean()
{
    bufRef_ = 0;
    delete doc_;
    doc_ = 0;
}



void TextRangeTest::testMoveCaret()
{
    bufRef_->appendText("regel 1.\n   leading whitespace\n   \n trailing  whitespace   \neof");

    TextRange range(0,0);
    range.setCaret(4);

    range.moveCaret(doc_,-1);
    testEqual( range.caret(), 3 );

    range.moveCaret(doc_,-2);
    testEqual( range.caret(), 1 );

    range.moveCaret(doc_,-2);
    testEqual( range.caret(), 0 );

    range.setCaret(doc_->length()-2);
    range.moveCaret(doc_,10);
    testEqual( range.caret(), bufRef_->length() );
}

void TextRangeTest::testMoveCaretWhileChar()
{
    bufRef_->replaceText(0,0,"  abacdx!");

    TextRange range(0,0);
    testEqual( range.caret(), 0 );

    range.moveCaretWhileChar( doc_, 1, "a" );
    testEqual( range.caret(), 0 );

    range.moveCaretWhileChar( doc_, 1, "\t x" );
    testEqual( range.caret(), 2 );

    range.moveCaretWhileChar( doc_, 1, "abc" );
    testEqual( range.caret(), 6 );

    range.moveCaretWhileChar( doc_, 1, "de xyz" );
    testEqual( range.caret(), 8 );

    range.moveCaretWhileChar( doc_, 1, "#!" );
    testEqual( range.caret(), 9 );
    testEqual( range.caret(), doc_->length() );


    range.moveCaretWhileChar( doc_, -1, "#!" );
    testEqual( range.caret(), 8 );

    range.moveCaretWhileChar( doc_, -1, "abcdx" );
    testEqual( range.caret(), 2 );

    range.moveCaretWhileChar( doc_, -1, "a c" );
    testEqual( range.caret(), 0 );

}

void TextRangeTest::testMoveCaretUntilChar()
{
    bufRef_->replaceText(0,0,"  abacdx!");

    TextRange range(0,0);
    testEqual( range.caret(), 0 );

    range.moveCaretUntilChar( doc_, 1, "bc" );
    testEqual( range.caret(), 3 );

    range.moveCaretUntilChar( doc_, 1, "xyz" );
    testEqual( range.caret(), 7 );

    range.moveCaretUntilChar( doc_, 1, "$" );
    testEqual( range.caret(), doc_->length() );

    range.moveCaretUntilChar( doc_, -1, "b" );
    testEqual( range.caret(), 4 );

    range.moveCaretUntilChar( doc_, -1, "$" );
    testEqual( range.caret(), 0 );

}



void TextRangeTest::testMoveCaretByCharGroup()
{
    bufRef_->replaceText(0,bufRef_->length(),"");
    bufRef_->replaceText(0,0,"something !! in @@ the ## water $$ does %% not ^^ compute\n   something!!in@@the##water$$does%%not^^compute");
    //                     0123456789012345678901234567890
    //"regel 1.|   leading whitespace|   | trailing  whitespace   |eof"

    TextRange range(0,0);
    QStringList sep("!@#$%^");
    QString ws("\n\t ");

    range.moveCaretByCharGroup( doc_, 1, ws, sep );
    testEqual( range.caret(), 9 );

    range.moveCaretByCharGroup( doc_, 2, ws, sep );
    testEqual( range.caret(), 15 );

    range.moveCaretByCharGroup( doc_, 1, ws, sep );
    testEqual( range.caret(), 18);

    range.moveCaretByCharGroup( doc_, -1, ws, sep );
    testEqual( range.caret(), 16);

    range.moveCaretByCharGroup( doc_, -2, ws, sep );
    testEqual( range.caret(), 10);
}



/// moves the caret to the line boundary
void TextRangeTest::testMoveCaretToLineBoundary()
{
    bufRef_->appendText("regel 1.\n   leading whitespace\n   \n trailing  whitespace   \neof");

    TextRange range(0,0);
    QString ws = "\t ";
    testEqual( range.caret(), 0 );

    // test move to EOL (normal line)
    range.moveCaretToLineBoundary( doc_, 1, ws );
    testEqual( range.caret(), 8 );

    range.moveCaretToLineBoundary( doc_, 1, ws );   // moving again should stay there
    testEqual( range.caret(), 8 );

    // test move to start of first word
    range.setCaret(9);  // second line
    range.moveCaretToLineBoundary( doc_, -1, ws );
    testEqual( range.caret(), 12 );

    range.moveCaretToLineBoundary( doc_, -1, ws );  // test movement to start of lie from first word
    testEqual( range.caret(), 9 );

    range.setCaret(14);  // second line
    range.moveCaretToLineBoundary( doc_, -1, ws );  // test movement to start of word from half line
    testEqual( range.caret(), 12 );

    // test eol specials
    int endDocPos = doc_->length();
    range.setCaret(endDocPos);
    range.moveCaretToLineBoundary( doc_, 1, ws );  // movement to the end at the end should stay at the end
    testEqual( range.caret(), endDocPos );

    range.moveCaretToLineBoundary( doc_, -1, ws );  // movement to the end at the end should stay at the end
    testEqual( range.caret(), 60 );

    range.moveCaretToLineBoundary( doc_, -1, ws );  // moving again should stay there
    testEqual( range.caret(), 60 );

    range.moveCaretToLineBoundary( doc_, 1, ws );  // And should move to the end again
    testEqual( range.caret(), endDocPos );

    range.set( endDocPos, endDocPos );
    range.moveCaretToLineBoundary( doc_, 1, ws );  // And should move to the end again
    testEqual( range.caret(), endDocPos );

}




//=================================================================================

/// initializes the test case
void TextRangeSetTest::init()
{
    controller_ = new TextEditorController();
    docRef_ = controller_->textDocument();
    bufRef_ = docRef_->buffer();
    selRef_ = dynamic_cast<TextRangeSet*>( controller_->textSelection() );
    selRef_->clear();   // remove all ranges
}

/// cleans the test case
void TextRangeSetTest::clean()
{
    delete controller_;
    selRef_= 0;
    docRef_ = 0;
    controller_ = 0;
}



/// This method tests the constructor of the selection
void TextRangeSetTest::testConstructor()
{
    testRanges("");  // caret and anchor should be together
}

/// This method test the overlapping ranges
void TextRangeSetTest::testAddRange()
{
    selRef_->addRange(0,0);
    selRef_->addRange(2,4);
    testRanges("0>0,2>4");

    // right 'overlap'  /////XX\\\\ .
    selRef_->addRange(3,6);
    testRanges("0>0,2>6");

    // overlap is ignored
    selRef_->addRange(6,4);
    testRanges("0>0,2>6");

    // check border conditions
    selRef_->addRange(6,10);
    testRanges("0>0,2>6,6>10");

    // replace the complete center part
    selRef_->addRange(8,1);
    testRanges("0>0,1>10");
}


/// test the range between offsets functions
void TextRangeSetTest::testRangesBetweenOffsets()
{
    addRanges( selRef_, "0>0,2>6,8>10" );

    int first=-1, last=-1;
    testFalse( selRef_->rangesBetweenOffsets( 12, 25, first, last ) );

    testTrue( selRef_->rangesBetweenOffsets( 9, 11, first, last ) );
    testEqual( first, 2 );
    testEqual( last, 2 );

    // test the 'border
    testTrue( selRef_->rangesBetweenOffsets( 10, 11, first, last ) );
    testEqual( first, 2 );
    testEqual( last, 2 );

    // mutliple ranges
    testTrue( selRef_->rangesBetweenOffsets( 3, 9, first, last ) );
    testEqual( first, 1 );
    testEqual( last, 2 );

    // position 0 should work
    testTrue( selRef_->rangesBetweenOffsets( 0, 4, first, last ) );
    testEqual( first, 0 );
    testEqual( last, 1);

}

/// This method 'tests' the movement of caters
void TextRangeSetTest::testMoveCarets()
{
    bufRef_->appendText("a1\nbb2\nccc3\n...\n");
    selRef_->addRange(0,0);    // reset the selection
    testRanges("0>0");

    // basic tests
    //------------

        // moving the caret to the right should work
        selRef_->moveCarets(1);
        testRanges("0>1");

        // try to move past the end
        selRef_->moveCarets(100);
        testRanges("0>16");

        // try move before the start of the document
        selRef_->moveCarets(-100);
        testRanges("0>0");

    // multiple cursor tests
    //----------------------

        selRef_->addRange(4,2);
        testRanges("0>0,4>2");

        selRef_->moveCarets(-1);
        testRanges("0>0,4>1");

        // check if the cursor is merged (the 0th cursor is special, because it cannot have any selection before it)
        selRef_->moveCarets(-1);
        testRanges("4>0");

        selRef_->resetAnchors();
        testRanges("0>0");

    // next test if rangings in the middle are placed next to eachother

        selRef_->addRange(4,6);
        selRef_->addRange(8,10);
        testRanges("0>0,4>6,8>10");

        selRef_->moveCarets(1);
        testRanges("0>1,4>7,8>11");

        // selections should only merge if they truely overlap
        selRef_->moveCarets(1);
        testRanges("0>2,4>8,8>12");

        selRef_->moveCarets(1);
        testRanges("0>3,4>13");


}

/// This method test that changing of the spatial information
void TextRangeSetTest::testChangeSpatial()
{
    //                #..|.[ ]| [   | ]..
    //===============[012|3456|78901|2345|]
    bufRef_->appendText("a1\nbb2\nccc3\n...\n");
    selRef_->addRange(0,0);
    // range (0,0)
    selRef_->addRange(4,6);
    selRef_->addRange(8,13);
    testRanges("0>0,4>6,8>13");

    // insert a single character at (0)
    selRef_->changeSpatial(0,0,1);
    testRanges("0>0,5>7,9>14");

    // The 'document' now is this: "?a1|bb2|ccc3|...\n");
    selRef_->changeSpatial(4,2,0);
    testRanges("0>0,4>5,7>12");


//    sel_->addRange(0,0);
//    sel_->changeSpatial(0, 1);

}


void TextRangeSetTest::testGetSelectedTextExpandedToFullLines()
{
    bufRef_->appendText("aaa\nbbb\ncccc");
    selRef_->addRange(0,0);
    testEqual( selRef_->getSelectedTextExpandedToFullLines(), "aaa\n");

    selRef_->setRange(1,1,0);
    testEqual( selRef_->getSelectedTextExpandedToFullLines(), "aaa\n");

    selRef_->setRange(3,1,0);
    testEqual( selRef_->getSelectedTextExpandedToFullLines(), "aaa\n");

    selRef_->setRange(4,4,0);
    testEqual( selRef_->getSelectedTextExpandedToFullLines(), "bbb\n");

    selRef_->setRange(4,2,0);
    testEqual( selRef_->getSelectedTextExpandedToFullLines(), "aaa\nbbb\n");
}



/// Test substract rnage
void TextRangeSetTest::testSubstractRange()
{
    bufRef_->appendText("hier moet een lange genoege tekst staan.");
    selRef_->addRange(2,5);
    selRef_->addRange(8,10);
    testRanges("2>5,8>10");

    // test empty range
    selRef_->substractRange(0,2);
    testRanges("2>5,8>10");

    selRef_->substractRange(4,9);
    testRanges("2>4,9>10");

    selRef_->substractRange(7,12);
    testRanges("2>4");

    selRef_->setRange(2,10,0);
    selRef_->substractRange(4,6);
    testRanges("2>4,6>10");

}




} // edbee