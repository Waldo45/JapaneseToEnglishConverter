/*
	Japanese to English Converter
	
	Objective: Translate word files with stories written in Japanese into English.
	
	Description: Translates text files from Japanese to English. Writing into English sentences
					is not complete and requires an additional program to combine the English translations
					into complete grammar correct sentences.
					
					Currently gives descriptors such as VERB, OBJECT, SUBJECT with the English translation.
					
	To Do: Make more modular. Use class structure to become more Object Oriented. Allow for multi-threading.
			Use a more expanded Japanese dictionary. Reduce lines of code to be shorter. Currently 700+.
*/


#include<iostream>
#include<fstream>
#include<string>
#include<string.h>
#include<ctype.h>
#include<cstdlib>
using namespace std;

//*****************************************************
//
//				GLOBAL VARIABLES AND ARRAYS
//
//*****************************************************

//Global File Stream variable so there is no need to pass to scanner
fstream fileStream;

//Updated tokentypes library.
enum tokentype {ERROR, WORD1, WORD2, PERIOD, VERB, VERBNEG, VERBPAST,VERBPASTNEG, IS, WAS, 
				OBJECT, SUBJECT, DESTINATION, PRONOUN, CONNECTOR, EOFM};

//Japanese and English Lexicon libraries.
string jlexicon[9] = {"watashi", "anata", "kare", "kanojo", "sore", "mata", "soshite", "shikashi", "dakara"};
string elexicon[9] = {"I/me", "you", "he/him", "she/her", "it", "Also", "Then", "However", "Therefore"};

//Japanese reserved words library
string reservedwords[] = 	{"masu", "masen", "mashita", "masendeshita", "desu", "deshita", "o", "wa", "ni", 
							"watashi", "anata", "kare", "kanojo", "sore", "mata", "soshite", "shikashi", "dakara", "eofm"};

//String array of the tokentype enum's to be used for printing purposes only
string tokentypearray[] = {"ERROR", "WORD1", "WORD2", "PERIOD", "VERB", "VERBNEG", "VERBPAST", "VERBPASTNEG", "IS", "WAS", "OBJECT", "SUBJECT", "DESTINATION", "PRONOUN", "CONNECTOR", "EOFM"};
							
//Saved Lexeme for parser functions interaction with scanner
string saved_lexeme;

//Saved Token for parser functions interaction with scanner
tokentype saved_token;

//boolean value to hold whether a next token is available
bool token_available = false;

//*****************************************************
//
//				Function Prototypes
//
//*****************************************************
//author: Bolen
//in order of appearance

void story();
void s();
void afterSubject();		//s1
void afterSubjectNoun();	//s2
void afterObject();			//s3

tokentype next_token();
bool match(tokentype t);

void noun();
void verb();
void be();
void tense();

void syntax_error1(tokentype);
void syntax_error2(string);

bool word_token(string s);
bool period_token(string s);

void scanner(tokentype& a, string& w);

							
//*****************************************************							
//								
//		STORY()		<story>::=<s>{<s>}
//
//*****************************************************
//author: Tian, Bolen
							
void story()
{
	cout << "Processing <story>" << endl;
	s();
	while(true)
	{
		switch(next_token())	// look ahead at next token
		{
			case CONNECTOR:
				s();
				break;
			default:
				return;
		}
	}
}						
							
//*****************************************************							
//								
//		S()     <s>::=[CONNECTOR]<noun>SUBJECT<s1>
//
//*****************************************************
//author: Patrick
							
void s()
{
	cout << "============Processing <s>============" << endl;
	switch(next_token())
	{
	case CONNECTOR:
	  match(CONNECTOR);
	case WORD1:
	case PRONOUN:
	  noun();
	  //case SUBJECT:
	  match(SUBJECT);
	  afterSubject(); //s1()
	  break;
	case EOFM:
		break;
	default:
	  syntax_error2("s");
	}
}	


//*****************************************************							
//								
//		afterSubject();    
//
//		<s1>::=	  <verb><tense>PERIOD 
//				| <noun><s2>
//
//*****************************************************
//author: Tian Bolen
void afterSubject()
{
	cout << "Processing <s1> 'afterSubject()'" << endl;
	switch(next_token())
	{
		case WORD2:
			verb();
			tense();
			match(PERIOD);
			s();
			break;
		case WORD1:
		case PRONOUN:
			noun();
			afterSubjectNoun(); //s2()
			break;
		default:
			syntax_error2("afterSubject <s1>");
	}
}

//*****************************************************							
//								
//		afterSubjectNoun();		
//
//		<s2>::=	  <be>PERIOD 
//				| DESTINATION<verb><tense>PERIOD 
//				| OBJECT <s3>
//
//*****************************************************
//author: Patrick
void afterSubjectNoun()		//s2
{
	cout << "Processing <s2> 'afterSubjectNoun()'" << endl;
	switch(next_token())
	{
	case IS:
	case WAS:
	  be();
	  match(PERIOD);
	  s();
	  break;
	case DESTINATION:
	  match(DESTINATION);
	  verb();
	  tense();
	  match(PERIOD);
	  s();
	  break;
	case OBJECT:
	  match(OBJECT);
	  afterObject();	//s3()
	  break;
	default:
	  syntax_error2("afterSubjectNoun <s2>");
	}
}

//*****************************************************							
//								
//		afterObject();		
//
//		<s3>::=   <verb><tense>PERIOD 
//				| <noun>DESTINATION<verb><tense>PERIOD
//
//*****************************************************
//author: Patrick
void afterObject()
{
	cout << "Processing <s3> 'afterObject()'" << endl;
	switch(next_token())
	{
	case WORD2:
	  verb();
	  tense();
	  match(PERIOD);
	  s();
	  break;
	case WORD1:
	case PRONOUN:
	  noun();
	  match(DESTINATION);
	  verb();
	  tense();
	  match(PERIOD);
	  s();
	  break;
	default:
	  syntax_error2("afterObject <s3>");
	}
}


//*****************************************************							
//								
//		NEXT_TOKEN()
//
//*****************************************************
//author: Tian, Bolen, Patrick							
//  Looks ahead to see what token comes next from the scanner.
tokentype next_token()
{
  //cout << "Processing next_token" << endl;
  if(!token_available)
    {
		scanner(saved_token, saved_lexeme); //get next token
		token_available = true;
    }
	return saved_token;
}							
							
							
//*****************************************************							
//								
//		MATCH(tokentype t)
//
//*****************************************************			
//author: Tian, Bolen, Patrick							
//Checks and eats up the expected token.
bool match(tokentype t)
{
  if(next_token() != t) // mismatch
	{
		syntax_error1(t);
	}
	else
	{
		token_available = false; // eat token
		cout << "Matched " << tokentypearray[t] << endl;
		if (tokentypearray[t] == "PERIOD")
		  cout << endl;
		return true;
	}
}	

//*****************************************************							
//								
//		NOUN()		<noun>::= WORD1 | PRONOUN
//
//*****************************************************
//author: Tian, Bolen							
void noun()
{
	cout << "Processing <noun>" << endl;
	switch(next_token())
	{
		case WORD1:
			match(WORD1);
			break;
		case PRONOUN:
			match(PRONOUN);
			break;
		default:
			syntax_error2("noun");
	}
}


//*****************************************************							
//								
//		VERB()		<verb>::= WORD2
//
//*****************************************************
//author: Tian, Bolen
void verb()
{
	cout << "Processing <verb>" << endl;
	switch(next_token())
	{
		case WORD2:
			match(WORD2);
			break;
		default:
			syntax_error2("verb");
	}
}



//*****************************************************							
//								
//		BE()		<be>::= IS | WAS
//
//*****************************************************
//author: Tian, Bolen
void be()
{
	cout << "Processing <be>" << endl;
	switch(next_token())
	{
		case IS:
			match(IS);
			break;
		case WAS:
			match(WAS);
			break;
		default:
			syntax_error2("be");
	}
}

//*****************************************************							
//								
//		TENSE()		
//
//		<tense>::= VERBPAST | VERBPASTNEG | VERB | VERBNEG
//
//*****************************************************
// author: Tian, Bolen
void tense()
{
	cout << "Processing <tense>" << endl;
	switch(next_token())
	{
		case VERBPAST:
			match(VERBPAST);
			break;
		case VERBPASTNEG:
			match(VERBPASTNEG);
			break;
		case VERB:
			match(VERB);
			break;
		case VERBNEG:
			match(VERBNEG);
			break;
		default:
			syntax_error2("tense");
	}
}


//*****************************************************							
//								
//		syntax_error1();  //when a match fails
//
//*****************************************************	
//author: Bolen, Tian, Patrick
void syntax_error1(tokentype expected)
{
	cout << "SYNTAX ERROR: expected " << tokentypearray[expected] << " but found " << saved_lexeme << "." << endl;
	exit(1);
}



//*****************************************************							
//								
//		syntax_error2();		//when a parser function fails
//
//*****************************************************	
//author: Tian, Bolen, Patrick
void syntax_error2(string parseFunction)
{
	cout << "SYNTAX ERROR: unexpected " << saved_lexeme << " found in " << parseFunction << "." << endl;
	exit(1);
}			
							
							
//*****************************************************							
//								
//		WORD_TOKEN(string s)
//
//*****************************************************							
							
//Done by: Tian, Bolen
bool word_token(string s)
{
// Initializing the state and character position to 0
	int state = 0;
	int charpos = 0;
  
	// Go through each character in the string and see if we can end in a final state; therefore meaning the string matches
	while (s[charpos] != '\0') 
	{
		char sc = tolower(s[charpos]); //lower case to allow I and E
		
		//Q0
		//states going out of q0
		if (state == 0 && (sc == 'd' || sc == 'j' || sc == 'w' || sc == 'y' || sc == 'z'))
			state = 1;
		else if (state == 0 && (sc == 'b' || sc == 'g' || sc == 'h' || sc == 'k' || sc 	== 'm' || sc == 'n' ||  sc == 'p' ||  sc == 'r'))
			state = 7;
		else if (state == 0 && sc == 's')
			state = 5;
		else if (state == 0 && sc == 't')
			state = 6;
		else if (state == 0 && sc == 'c')
			state = 2;	
		else if (state == 0 && (sc == 'a' || sc == 'e' || sc == 'i' || sc == 'o' || sc 	== 'u'))
			state = 9;
		
		//Q1
		//states going out of q1
		else if (state == 1 && (sc == 'a' || sc == 'e' || sc == 'i' || sc == 'o' || sc 	== 'u'))
			state = 9;
		
		//Q2
		//states going out of q2
		else if (state == 2 && sc == 'h')
			state = 3;
		
		//Q3
		//states going out of q3
		else if (state == 3 && (sc == 'a' || sc =='e' || sc == 'i' || sc == 'o' || sc == 'u'))
		  state = 9;
	  
		//Q4
		//states going out of q4
		else if (state == 4 && (sc == 'a' || sc == 'e' || sc == 'i' || sc == 'o' || sc == 'u'))
		  state = 9;
	  
		//Q5
		//states going out of q5
		else if (state == 5 && sc == 'h')
		  state = 3;
		else if (state == 5 && ( sc == 'a' || sc == 'e' || sc == 'i' || sc == 'o' || sc == 'u'))
		  state = 9;
	  
		//Q6
		//states going out of q6
		else if (state == 6 && sc == 's')
		  state = 3;
		else if (state == 6 && (sc == 'a' || sc == 'e' || sc == 'i' || sc == 'o' || sc == 'u'))
		  state = 9;
	  
		//Q7
		//states going out of q7
		else if (state == 7 && sc == 'y')
		  state = 4;
		else if (state == 7 && (sc == 'a' || sc =='e' || sc == 'i' || sc == 'o' || sc == 'u'))
		  state = 9;
	  
		//Q8
		//states going out of q8
		else  if (state == 8 && (sc == 'a' || sc == 'e' || sc == 'i' || sc == 'o' || sc == 'u'))
		  state = 9;
		else if (state == 8 && (sc == 'd' || sc == 'j' || sc == 'w' || sc == 'y' || sc == 'z'))
		  state = 1;
		else if (state == 8 && (sc == 'b' || sc == 'g' || sc == 'h' || sc == 'm' || sc == 'n' || sc == 'p' || sc == 'r'))
		  state = 7;
		else if (state == 8 && (sc == 'k' || sc == 's'))
		  state = 5;
		else if (state == 8 && sc == 'c')
		  state = 2;
		else if (state == 8 && sc == 't')
		  state = 6;
	  
		//Q9
		//states going out of q9
		else if (state == 9 && sc == 'c')
		  state = 2;
		else if (state == 9 && sc == 't')
		  state = 6;
		else if (state == 9 && sc == 's')
		  state = 5;
		else if (state == 9 && (sc == 'd' || sc == 'w' || sc == 'y' || sc == 'z'))
		  state = 1;
		else if (state == 9 && sc == 'n' )
		  state = 10;
		else if (state == 9 && (sc == 'a' || sc == 'e' || sc == 'i'  || sc == 'o' || sc == 'u'))
		  state = 9;
		else if (state == 9 && (sc == 'b' || sc == 'g' || sc == 'h'  || sc == 'k' || sc == 'm' || sc =='p' || sc == 'r'))
		  state = 7;
	  
		//Q10
		//states going out of q10 
		else if (state == 10 && sc == 'y')
		  state = 8;
		else if (state == 10 && sc == 't')
		  state = 6;
		else if (state == 10 && (sc == 'a' || sc == 'e' || sc == 'i'  || sc == 'o' || sc == 'u'))
		  state = 9;
		else if (state == 10 && sc == 's')
		  state = 5;
		else if (state == 10 && (sc == 'b' || sc == 'g' || sc == 'h'  || sc == 'k' || sc == 'm' || sc == 'n' || sc =='p' || sc == 'r'))
		  state =7;
		else if (state == 10 && (sc == 'd' ||  sc == 'j' || sc == 'w' || sc == 'z'))
		  state = 1;
		else
		{
			return(false);
		}
		
		charpos++;
		
		//this checks to see if the charpos exceeds the size of the string - 1
		if (charpos > s.size()-1)
			break;
		
	}//end of while loop
	
	// if I ended in a final state then return true;
	// else return false
	if (state == 0 || state == 8 || state == 9 || state == 10)
		return(true);  // end in a final state
	else
		return(false);
}




//*****************************************************							
//								
//		PERIOD_TOKEN(string s)
//
//*****************************************************

//Done by: Tian
bool period_token(string s)
{
	// initializing our state and character position to 0
	int state = 0;
	int charpos = 0;
		
	// checks to see if the next character is a PERIOD
	while (s[charpos] != '\0')
	{
		if (state == 0 && s[charpos] == '.')
			state = 1; //are in q1
		else
		{
			return(false);
		}
		
		charpos++;
			
		//this checks to see if the charpos exceeds the size of the  
		// string - 1
		
		if (charpos > s.size()-1)
			break;
		
	}//end of while loop

	// if I ended in a final state then return true;
	// else return false
	if (state == 1) 
		return(true);
	else 
		return(false);
}





//*****************************************************							
//								
//		SCANNER(tokentype& a, string& w)
//
//*****************************************************

//Scanner processes only one word each time it is called
//Done by: Tian
void scanner(tokentype& a, string& w)
{
/* 
	1. Call the token functions one after another and generate 
		a lexical error if both DFAs failed.
		Let the token_type be ERROR in that case.
	2. Make sure WORDs are checked against the reservedwords list. 
		If not reserved, token_type is WORD1 or WORD2.
	3. Return the token type & string  (pass by reference)
*/
  
	cout << "Called Scanner..." << endl;

	//assign Token Type to its string 
	fileStream >> w; //new word
	
	if (w == "masu")
		a = VERB;
	else if (w == "masen")
		a = VERBNEG;
	else if (w == "mashita")
		a = VERBPAST;
	else if (w == "masendeshita")
		a = VERBPASTNEG;
	else if (w == "desu")
		a = IS;
	else if (w == "deshita")
		a = WAS;
	else if (w == "o")
		a = OBJECT;
	else if (w == "wa")
		a = SUBJECT;
	else if (w == "ni")
		a = DESTINATION;
	else if (w == "watashi" || w == "anata" || w == "kare" || w == "kanojo" || w == "sore" )
		a = PRONOUN;
	else if (w == "mata" || w == "soshite" || w == "shikashi" || w == "dakara")
		a = CONNECTOR;
	else if (w == "eofm")
		a = EOFM;
	else if (word_token(w))
	{ 
		if (w[w.size()-1]== 'E' || w[w.size()-1] == 'I')
			a = WORD2;
		else 
			a = WORD1;
	}

	//call period(w) 
	else if (period_token(w))
	{
		a = PERIOD;
	}
	else //none of the FAs returned TRUE
	{
		cout << "--->LEXICAL ERROR: The string: " << w << " is not in my language." << endl;
		a = ERROR;
	}
}//the end




//*****************************************************							
//								
//		MAIN PROGRAM
//
//*****************************************************
//The test driver to call the scanner repeatedly  
//Done by:  Bolen
int main()
{
/*
    1. Get the input file name from the user
*/
	
	string fileName;
	cout 	<< "Please enter a file to read from: ";
	cin 	>> fileName;
	cout 	<< endl;
  
	cout 	<< "Opening: " << fileName << endl;

/*
    2. Open the input file which contains a story written in Japanese (fin.open).
*/
	
	fileStream.open(fileName.c_str());
  
	if(fileStream.is_open())
    {
		//PART B
		story();
	}
	
/*
	3. Close the input file
*/
	fileStream.close();
	
	cout << "End of File reached. Stopping." << endl;
	
	return 0;
	
}//end
