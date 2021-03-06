#include <iostream>
#include "automata.h"

using namespace std;

/**
 * Constructor for Abstract DFA.
 * 
 * @param noStates
 *            Number of states in the DFA.
 */
AbstractDFA::AbstractDFA(int noStates):current_state(0), trap_state(-1) {}
//Ho inizializzato lo stato iniziale allo stato 0

/**
 * Reset the automaton to the initial state.
 */
void AbstractDFA::reset() {

    /*
    Imposto lo stato corrente a 0
    */
    current_state = 0;
}

/**
 * Performs one step of the DFA for a given letter. If there is a transition
 * for the given letter, then the automaton proceeds to the successor state.
 * Otherwise it goes to the sink state. By construction it will stay in the
 * sink for every input letter.
 * 
 * @param letter
 *            The current input.
 */
void AbstractDFA::doStep(char letter) {
    /*
    Cerco nelle possibili transizioni dello stato corrente
    la transizione per il prossimo stato   
    */
    current_state = states[tpair(current_state,letter)];
}

/**
 * Check if the automaton is currently accepting.
 * 
 * @return True, if the automaton is currently in the accepting state.
 */
bool AbstractDFA::isAccepting() {
    /* 
    In un DFA l' automa accetta solo se si trova in uno stato finale,
    quindi controllo se mi trovo in uno stato finale
    */
   return std::count(final_states.begin(),final_states.end(),current_state);
}

/**
 * Run the DFA on the input.
 * 
 * @param inputWord
 *            stream that contains the input word
 * @return True, if if the word is accepted by this automaton
 */
bool AbstractDFA::run(const string &inputWord) {
    this->reset();
    for(int i = 0; i < inputWord.length(); i++) {
        doStep(inputWord[i]);
    }
    return isAccepting();
}

//funzioni ausiliarie

//aggiunge o modifica la transizione da  (FROM, INPUT) a TO
void AbstractDFA::manageTransition(const int& from, const char& input, int to){
    states[tpair(from, input)] = to;
}
void AbstractDFA::addFinalState(const int& id){ final_states.push_back(id); }

void AbstractDFA::setTrap(const int & id){
    trap_state = id;
}

int AbstractDFA::getTrap(){return trap_state;}

/**
 * Construct a new DFA that recognizes exactly the given word. Given a word
 * "foo" the constructed automaton looks like: -> () -f-> () -o-> () -o-> []
 * from every state (including the final one) every other input letter leads
 * to a distinguished sink state in which the automaton then remains
 * 
 * @param word
 *            A String that the automaton should recognize
 */
WordDFA::WordDFA(const string &word) : AbstractDFA(0) {


    //Utilizzo lo stato trappola per mantenere la correttezza del dfa.
    //In questo caso lo setto con il valore id di uno stato in pi?? rispetto
    //al totale di stati che sar?? quanto la parola pi?? lo stato trappola
    setTrap(word.length()+1);


    //Genero tutti gli stati del DFA e li faccio puntare di default 
    //con tutte le lettere dell'alfabeto allo stato trappola
    for(auto& lt : word){
        for(unsigned int i = 0; i<= word.length()+1; i ++){
            manageTransition(i,lt,getTrap());
        }
    }

    //Modifico lo stato corretto per tutte le transizioni che ne hanno bisogno
    for(unsigned int i = 0; i< word.length();i++){
        manageTransition(i,word[i],i+1);
    }

    //Aggiungo l'id dello stato finale alla lista degli stati finali cos?? che l'automa possa riconoscerlo.
    //L'id dello stato finale ?? la dimensione della parola
    addFinalState(word.length());    

    //Rappresentazione del DFA che riconosce la parola
    /*

        |\
        | \       
        |  (0) -- r[0] --> (1) -- e[1] --> (2)  -- p[2]--> (3) -- e[3] --> (n) -- ... --> (n-1) -- n-esima lettera --> ((n))
        | /
        |/
    
    */
}

/**
 * Construct a new DFA that recognizes comments within source code. There
 * are three kinds of comments: 
 * single line comment that starts with // and ends with a newline,
 * multiline comments that starts with (* and ends with *),
 * and multiline comments that starts with { and ends with }
 */
CommentDFA::CommentDFA() : AbstractDFA(0) {

    


    
    //Rappresentazione del DFA che riconosce i commenti
    /*
    
                                      "ANY"
    |\                                 / \
    | \                                \ v
    |  (0)----- "/" --> (1) -- "/" --> (2) -- "\n" -----------
    | / |\                                                   |
    |/  | \            "ANY"                                 |
        |  \            / \                                  |
        |   \           \ v                                  v
        |    -- "{" --> (4) ------------------ "}" ------> ((3))           
        |                                                    ^
        |                             "ANY"                  |
        |                              / \                  ")"
        |                              \ v                   |
        ------- "(" --> (5) -- "*" --> (6) -- "*" --> (7) ----
                                        ^               |
                                        |               |
                                         ---- "ANY" ----
    
    */

    //Qui l'automa ?? fissato e costruito appositamente per riconoscere i 3 tipi di commenti


    //Setto lo stato trappola allo stato 8, ovvero uno in pi?? dello stato pi?? grande dell'automa
    setTrap(8);

    //Setto lo stato finale allo stato 3
    addFinalState(3);

    //Genero tutti gli stati del DFA e li faccio puntare di default 
    //con tutte le lettere dell'alfabeto allo stato trappola
    for(auto& st : {0,1,2,3,4,5,6,7, AbstractDFA::getTrap()}){
        for(auto& lt : ALPHABET){
            manageTransition(st,lt,getTrap());
        }
    }

    //Setto tutte le varie transizioni dell'automa
    //Ramo che riconosce il commento monolinea "//..."
    manageTransition(0,'/',1);
    manageTransition(1,'/',2);
    manageTransition(2,ANY,2);
    manageTransition(2,'\n',3);
    
    //Ramo che riconosce il commento multilinea "{...}"
    manageTransition(0,'{',4);
    manageTransition(4,ANY,4);
    manageTransition(4,'}',3);

    //Ramo che riconosce il commento multilinea "(*...*)"
    manageTransition(0,'(',5);
    manageTransition(5,'*',6);
    manageTransition(6,ANY,6);
    manageTransition(6,'*',7);
    manageTransition(7,ANY,6);
    manageTransition(7,')',3);

}

/**
 * Performs one step of the DFA for a given letter. This method works
 * differently than in the superclass AbstractDFA.
 * 
 * @param letter
 *            The current input.
 */
void CommentDFA::doStep(char letter) {


    // chiamo il doStep di default
    AbstractDFA::doStep(
        // E gli passo:
        // - Se trovo il carattere nel mio alfabeto, uso quel carattere, 
        // - altrimenti uso ANY per rappresentarlo
        
        std::count(CommentDFA::ALPHABET.begin(), CommentDFA::ALPHABET.end(), letter) ? letter : CommentDFA::ANY
    );
}	


