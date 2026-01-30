#include <iostream>
#include <unordered_map>
#include <vector>
#include <string>
#include <fstream>

std::unordered_map<std::string, int> lv_key;//local have different key than global
std::unordered_map<std::string, int> gv_key;
std::unordered_map<int,int> lKey_index;
std::unordered_map<int,int> gKey_index;
std::unordered_map<std::string,int> operator_key = {{"G",0},{"FG",1},{"F",2},{"X",3},{"U",4},{"NOT",5},{"AND",6},{"OR",7}};
std::unordered_map<int,std::string> key_operator = {{0,"G"},{1,"FG"},{2,"F"},{3,"X"},{4,"U"},{5,"NOT"},{6,"AND"},{7,"OR"}};
int *lv_values;
int *gv_values;
int lvSize = 0, gvSize = 0;
std::vector<std::string> actions;

class formula{

   std::vector<formula*> parts;
   std::vector<int> operators;
   std::vector<int> variables;
   std::vector<int> xPrepared;
   std::vector<int> xTrue;
   int xInd = 0;
   std::vector<int> uRes;
   std::vector<int> fTrue;
   int fInd = 0, uInd = 0;
   std::vector<int> gFalse;
   int gInd = 0;
   int lastAction = 0;
   int fixed, value;
   int TrueConstant = 0, FalseConstant=0;

    public: 

    formula(){
        fixed = value = 0;
    }

    void setLastAction(){
       lastAction = 1;
    }

    formula(std::string f){
        fixed = value = 0;
        std::string op="";
        int ind  = 0;
        if(f[0] == '('){//case ( FG (v1 op1 v2 op2 .... vk )) or ( FG ((v1 op 1 v2 op ... vk) op () ... op ())
           if(f[2] == 'F'){
                 op+="F";
                 if(f[3] == 'G'){
                    if(f[4] == ' '){
                      op+="G";
                      ind = 4;
                    }
                 }
                 else if(f[3] == ' ') ind = 3;
           }
           else if(f[2] == 'G'){
                  op+="G";
                  if(f[3] == 'F'){
                    if(f[4] == ' '){
                      op+="F";
                      ind = 4;
                    }
                  }
                  else if(f[3] == ' ') ind = 3;
           }
           else if(f[2] == 'X'){
                    op+="X";
                    ind = 3;
           }
           else if(f[2] == 'N' && f[3] == 'O' && f[4] == 'T'){
                  op+="NOT";
                  ind = 5;
           }

           if(ind == 3 || ind == 4 || ind == 5){
            //std::cout<<"OP1: "<<op<<std::endl;
            if(op!=""){
             // std::cout<<"op: "<<op<<std::endl;
              //std::cout<<operator_key[op]<<std::endl;
           operators.push_back(operator_key[op]);
            }
           std::string subf = "";
           for(int i=ind+2; i<f.size()-2;i++)
                subf+=f[i];
          //  std::cout<<"Subformula: "<<subf<<std::endl;
            formula *fs = new formula(subf);
            parts.push_back(fs);
           }
           else{//case (v1 op1 v2 op2 .... vk ) op () .... ()

             if(f[2] == '('){ //case ( subf ) op ( subf ) etc.
          
           // std::cout<<"Novi case"<<std::endl;
          //  std::cout<<f<<std::endl;
           // return;
            std::string tmp = "";
            int parentCount = 0, endIndex = 0, startIndex = 0;
            while(endIndex <f.size()){
                for(int i=startIndex;i<f.size();i++){
                        if(f[i] == '(') parentCount++;
                        if(i>startIndex && parentCount == 0){
                             endIndex = i;
                             break;
                        }
                        if(f[i] == ')') parentCount--;
                        if(i>startIndex && parentCount>0) tmp+=f[i];
                        if(i == f.size()-1) endIndex = i;
                }
               
                std::string tmp1="";
                int p = 0, kk;

                for(kk=0;kk<tmp.size();kk++){
                   if(tmp[kk] == '(') p++;
                   if(tmp[kk] == ')') p--;
                   if(kk>0 && p == 0) break;
                }


                //std::cout<<"tmp: "<<tmp<<std::endl;;

                for(int l = 2; l<tmp.size();l++)
                      if(l!=kk) tmp1+=tmp[l];

                      
                 std::cout<<"tmp1: "<<tmp1<<std::endl;     

               formula *fs = new formula(tmp1);
                    parts.push_back(fs);
                std::cout<<"Part added"<<std::endl;
                  std::string subf="";
                  endIndex++;
                  if(endIndex >= f.size()) break;
                    while(f[endIndex]!=' '){
                        subf+=f[endIndex];
                        endIndex++;
                    }


                    

                    //std::cout<<"Operator: "<<subf<<std::endl;
                    operators.push_back(operator_key[subf]);

                    startIndex = endIndex+1;
                    //std::cout<<"index - letter: "<<startIndex<<" "<<f[startIndex]<<std::endl;
                    parentCount = 0;
                    tmp = "";
                    tmp1 = "";
            }
                    return;
           }
            else if(f[0] == '(' && f[1] == '('){ //case ((v1 op1 v2 op20 op () ... ()))
                        std::string tmp = "";
                        for(int i=1;i<f.size()-1;i++)
                             tmp+=f[i];
                      f = tmp;
              }

             std::string subf = "";
             std::cout<<"TEST: "<<f<<std::endl;
             for(int i=1;i<f.size()-1;i++){
                if(f[i]!=')' && f[i]!='(')
                subf+=f[i];
                else if(f[i] == '(') continue;
                else{
                    formula *fs = new formula(subf);
                    parts.push_back(fs);
                    subf = "";
                    if(i == f.size()-1) break;
                    int j = i+2;
                    while(f[j]!=' '){
                        subf+=f[j];
                        j++;
                    }
                    //std::cout<<"OP2: "<<subf<<std::endl;
                    if(subf!=""){
                      //std::cout<<"op1: "<<subf<<std::endl;
                    operators.push_back(operator_key[subf]);
                    }
                    subf = "";
                }
             }

           }

        }
        else{//v1 op1 v2 op2 ... OR NOT (v1) opt1 NOT (v2) opt ...

                if(f == "TRUE"){
                     TrueConstant = 1;
                     return;
                }
                else if(f == "FALSE"){
                     FalseConstant = 1;
                     return;
                }
           
               if(lv_key.find(f) != lv_key.end()){//only vi
                       variables.push_back(lv_key[f]);
                       return;
               }
               else if(gv_key.find(f) != gv_key.end()){//onlly vi
                      variables.push_back(gv_key[f]);
                       return;
               }

               std::string subf = ""; //more complex case
               int i=0;
               if(f[0] == ' ') i=1;
               for(;i<f.size();i++){
                        while(f[i]!=' ' && i<f.size()){
                             subf+=f[i];
                             i++;
                        }
                         //std::cout<<"Complex: "<<f<<std::endl;
                        // std::cout<<"Complex subf: "<<subf<<std::endl;
                        if(subf == "NOT" || subf == "AND" || subf == "OR"){
                          while(i<f.size()){
                          //std::cout<<"OP3: "<<subf<<std::endl;
                          if(subf!=""){
                            //std::cout<<"op3: "<<subf<<std::endl;
                           operators.push_back(operator_key[subf]);
                          }
                           subf = ""; i++;
                           int c = 0;
                           if(f[i] == '('){ i++; c++;}
                           while(f[i]!=')' && i<f.size()){
                            if(f[i] == '(') c++;
                             subf+=f[i];
                             i++;
                        }
                        if(c == 2) subf+=")";
                        
                        //std::cout<<"Subformula for recursion: "<<subf<<std::endl;
                           formula *fs = new formula(subf);
                            parts.push_back(fs);
                            subf = "";
                        //  std::cout<<"Fprint: "<<f[i]<<std::endl;
                          while(f[i] == ')') i++;
                          i++;
                          while(f[i]!=' ' && i<f.size()){
                             subf+=f[i];
                             i++;
                        }
                         //  std::cout<<"OperatorLater: "<<subf<<std::endl;
                        if(i == f.size()) return;

                       }
                   }
                        else{            

                        if(lv_key.find(subf) != lv_key.end()){
                            // variables.push_back(lv_key[subf]);
                            formula *fs = new formula(subf);
                            parts.push_back(fs);
                            subf = "";
                        }
                        else if(gv_key.find(subf) != gv_key.end()){
                             //variables.push_back(gv_key[subf]);
                             formula *fs = new formula(subf);
                            parts.push_back(fs);
                            subf = "";
                          } 
                        }
                        subf = "";  
                        
                        if( i == f.size()-1) break;

                       while(f[i]!=' ' && f[i] !=')' && i<f.size()){
                             subf+=f[i];
                             i++;
                        }
                        //std::cout<<"OP4: "<<subf<<std::endl;
                        if(subf!=""){
                          std::cout<<"op4: "<<subf<<std::endl;
                        operators.push_back(operator_key[subf]);
                        }

               }
        }

            //starts with (
               //contains F, G, FG or GF
               //contains up to k variables and k-1 operators -> potentially following operators and other groups
            //starts with a variable -> following operators and variables
     }
    
     int evaluate(){//watch on literal level negation

      if(parts.empty()){ //assumption, at least 2 variables and one operator

        if(TrueConstant) return 1;
        if(FalseConstant) return 0;

        if(variables.size()==1){
          int varA = variables[0];
          int ind, val1;
           if(lKey_index.find(varA) != lKey_index.end()){//only vi
                         ind = lKey_index[varA];
                         val1 = lv_values[ind];
               }
               else if(gKey_index.find(varA) != gKey_index.end()){//onlly vi
                      ind = gKey_index[varA];
                      val1 = gv_values[ind];
               }

              return val1; 
        }

         int truth = 0;
         int varA = variables[0];
         int varB = variables[1];

         int ind, val1, val2;

           if(lKey_index.find(varA) != lKey_index.end()){//only vi
                         ind = lKey_index[varA];
                         val1 = lv_values[ind];
               }
               else if(gKey_index.find(varA) != gKey_index.end()){//onlly vi
                      ind = gKey_index[varA];
                      val1 = gv_values[ind];
               }

            if(lKey_index.find(varB) != lKey_index.end()){//only vi
                         ind = lKey_index[varB];
                         val2 = lv_values[ind];
               }
               else if(gKey_index.find(varB) != gKey_index.end()){//onlly vi
                      ind = gKey_index[varB];
                      val2 = gv_values[ind];
               }   

         std::string op = key_operator[operators[0]];
         if(op == "AND"){

             truth = val1 && val2;
             return truth;
         }
         else if(op == "OR"){
             truth = val1 || val2;
             return truth;
         }
         else if(op == "U"){
           if(uRes.size() > uInd){
                if(uRes[uInd] == 1)
                  return 1;
                else if(uRes[uInd] == -1)
                return 0;
           } 
           else {
                  if(!val1 && !val2){ //left false, but right still false -> until is false
                    uRes.push_back(-1);
                    uInd++;
                    return 0;
                  }
                  if(val1 && !val2) return 1; //left true, but right still false
                  if(val2){ //val2 is true, val1 not reported false before the change -> until true
                    uRes.push_back(1);
                    uInd++;
                    return 1; 
                  }
           }
         }
      }
      else{//case NOT (X, F, G, FG) (v1) op1 v2 op2 NOT (v2) op3 v3 op4 ...
             int truth = 0, partsInd = 0;
        
            
           for(int i=0;i<operators.size();i++) {
             std::string op = key_operator[operators[i]];
             

              if(op == "NOT"){//oblik NOT (subf) op2 ()....
                   truth = parts[partsInd]->evaluate();
                   truth = 1 - truth;
                   partsInd++;
              }
              else if(op == "X"){//provjeriti u sljedecem stanju
                  if(xPrepared.size()<=xInd){
                     xPrepared.push_back(1); xInd++;
                     truth = 1; //evaluiraj ostatak formule
                  }
                  else{
                     if(xTrue.size()<=xInd){
                            truth = parts[partsInd]->evaluate();
                            xTrue.push_back(truth);
                            xInd++;
                            partsInd++;
                     }
                     else{
                            truth = xTrue[xInd];
                     }
                  }   
              }
              else if(op == "F"){
                   if(fTrue.size()<=fInd){
                   truth = parts[partsInd]->evaluate();
                   partsInd++;
                   if(truth == 1){
                     fTrue.push_back(1); fInd++;
                   }
                 }
                 else truth = 1; //istinit u nekom prethodnom trenutku
              }
              else if(op == "G"){
                    if(gFalse.size()<=gInd){
                          truth = parts[partsInd]->evaluate();
                          partsInd++;
                          if(truth == 0){
                            gFalse.push_back(1); gInd++;
                          }
                    }
                    else truth = 0; //nekada laz
              }
              else if(op == "FG"){
                      if(lastAction == 1){
                            truth = parts[partsInd]->evaluate();
                            partsInd++;
                      }
                      else truth = 1; //dopusti evaluaciju ostatka formule
              }
              else if(op == "AND"){

                    //  std::cout<<"broj dijelova formule: "<<parts.size()<<std::endl;

                      if(parts.size() > 1 && i==0){
                        truth = parts[partsInd]->evaluate();
                        partsInd++;
                      }

                      if(i+1<<operators.size()){
                        if(key_operator[operators[i+1]] == "NOT"){
                              truth  = truth && (1-parts[partsInd]->evaluate());
                              partsInd++; i++;
                        }
                        else{
                               truth = truth && parts[partsInd]->evaluate();
                               partsInd++;
                        }
                      }
                      else{//nema operatora iza AND
                              truth = truth && parts[partsInd]->evaluate();
                              partsInd++;     
                      }
              }
              else if(op == "OR"){

                     if(parts.size() > 1 && i==0){
                        truth = parts[partsInd]->evaluate();
                        partsInd++;
                      }

                     if(i+1<<operators.size()){
                        if(key_operator[operators[i+1]] == "NOT"){
                              truth  = truth || (1-parts[partsInd]->evaluate());
                              partsInd++; i++;
                        }
                        else{
                               truth = truth || parts[partsInd]->evaluate();
                               partsInd++;
                        }
                      }
                      else{//nema operatora iza OR
                              truth = truth || parts[partsInd]->evaluate();
                              partsInd++;     
                      }
              }
              else if(op == "U"){
                int t1 = 0;
                if(i+1<<operators.size()){
                        if(key_operator[operators[i+1]] == "NOT"){//f U NOT (f1)
                              t1 = (1-parts[partsInd]->evaluate());
                              partsInd++; i++;

                              if(uRes.size() > uInd){
                                    if(uRes[uInd] == 1)
                                          truth = 1;
                                    else if(uRes[uInd] == -1)
                                           truth = 0;
                               } 
                             else {
                                     if(!truth && !t1){ //left false, but right still false -> until is false
                                             uRes.push_back(-1);
                                             uInd++;
                                         truth = 0;
                                 }
                                   // if(truth && !t1)  //left true, but right still false
                                    if(t1){ //val2 is true, val1 not reported false before the change -> until true
                                       uRes.push_back(1);
                                       uInd++;
                                      truth = 1;
                                  }
                            }                         
                        }
                        else{ //f U f1
                          t1 = parts[partsInd]->evaluate();
                          partsInd++; i++;

                          if(uRes.size() > uInd){
                                    if(uRes[uInd] == 1)
                                          truth = 1;
                                    else if(uRes[uInd] == -1)
                                           truth = 0;
                               } 
                             else {
                                     if(!truth && !t1){ //left false, but right still false -> until is false
                                             uRes.push_back(-1);
                                             uInd++;
                                         truth = 0;
                                 }
                                   // if(truth && !t1)  //left true, but right still false
                                    if(t1){ //val2 is true, val1 not reported false before the change -> until true
                                       uRes.push_back(1);
                                       uInd++;
                                      truth = 1;
                                  }
                            } 
                        }
                      }
              }
            }
                xInd = fInd = gInd = uInd = 0;
                return truth;  
      } 
     }

     void print(int space){

       if(TrueConstant) std::cout<<"TRUE"<<std::endl;
       else if(FalseConstant) std::cout<<"FALSE"<<std::endl;
          
         // std::cout<<"OS: "<<operators.size()<<std::endl;
          if(operators.size()>0){

             std::string op = key_operator[operators[0]];
             int partInd = 0;

         if(op == "NOT" || op == "G" || op == "F" || op == "FG" || op == "X"){ 
              for(int i=0;i<=space;i++) std::cout<<" ";
              std::cout<<op<<std::endl;
              parts[0]->print(space+1);
              partInd = 1;
         }
         else{
             parts[0]->print(space+1);
             for(int i=0;i<=space;i++) std::cout<<" ";
             std::cout<<op<<std::endl;
             parts[1]->print(space+1);
             partInd = 2;
         }

            for(int i=1;i<operators.size();i++){
              std::string op = key_operator[operators[i]];
                  if(op == "NOT" || op == "G" || op == "F" || op == "FG" || op == "X"){ 
              for(int i=0;i<=space;i++) std::cout<<" ";
              std::cout<<op<<std::endl;
              parts[partInd++]->print(space+1);
                   }
                   else{
                        for(int i=0;i<=space;i++) std::cout<<" ";
                        std::cout<<op<<std::endl;
                        parts[partInd++]->print(space+1);
                   }
            }
          }
          else if(variables.size() == 1){
            for(int i=0;i<=space;i++) std::cout<<" ";
            std::string variable = "";
              for (std::unordered_map<std::string,int>::const_iterator it = lv_key.begin(); it != lv_key.end(); ++it) {
                     if (it->second == variables[0]) variable = it->first;
                 }
              
              if(variable == ""){
                 for (std::unordered_map<std::string,int>::const_iterator it = gv_key.begin(); it != gv_key.end(); ++it) {
                     if (it->second == variables[0]) variable = it->first;
                 }
              }
              for(int i=0;i<=space;i++) std::cout<<" ";
              std::cout<<variable<<" ";
       }

     }
 
};

std::vector<formula*> local_formulas;
std::vector<formula*> global_formulas;

std::unordered_map<std::string,std::unordered_map<std::string,formula*>> gammaMinus;
std::unordered_map<std::string,std::unordered_map<std::string,formula*>> gammaPlus;

void readProblem(std::ifstream &input){
         //read propositions (local/global)
         int key = 0, lind = 0, gind = 0; 
         std::string s;
         int line = 1;
        while (std::getline(input,s))
         {
             std::cout<<s<<std::endl;
             if(line == 1){
              std::string tmp="";
              for(int i=0;i<s.size();i++){
                  if(s[i]!=','){
                      tmp+=s[i];
                      if(i == s.size()-1){
                        lv_key[tmp] = key;
                        lKey_index[key] = lind;
                        key++; lind++; i++;
                         tmp = "";
                      }
                  }
                  else if(s[i] == ','){//add to structures
                       lv_key[tmp] = key;
                       lKey_index[key] = lind;
                       key++; lind++; i++;
                        tmp = "";
                  }
                 
                }
             }
             else if(line == 2){
              std::string tmp="";
              for(int i=0;i<s.size();i++){
                  if(s[i]!=','){
                      tmp+=s[i];
                      if(i == s.size()-1){
                        gv_key[tmp] = key;
                        gKey_index[key] = gind;
                        key++; gind++; i++;
                        tmp = "";
                      }
                  }
                  else if(s[i] == ','){//add to structures
                       gv_key[tmp] = key;
                       gKey_index[key] = gind;
                       key++; gind++; i++; 
                       tmp = "";
                  }
                }
                
             }
             else if(line == 3){
               lv_values = new int[lind];
               gv_values = new int[gind];
               lvSize = lind;
               gvSize = gind;
                   
                   std::string tmp=""; int ind = 0;
              for(int i=0;i<s.size();i++){
                  if(s[i]!=','){
                      tmp+=s[i];
                      if(i == s.size()-1){
                        if(tmp == "TRUE")
                          lv_values[ind] = 1;
                       else lv_values[ind] = 0;
                       ind++; i++;  tmp = "";
                      }
                  }
                  else if(s[i] == ','){//add to structures
                       if(tmp == "TRUE")
                          lv_values[ind] = 1;
                       else lv_values[ind] = 0;
                       ind++; i++;
                       tmp = "";
                  }
                }
                 
             }
             else if(line == 4){
                    std::string tmp=""; int ind = 0;
              for(int i=0;i<s.size();i++){
                  if(s[i]!=','){
                      tmp+=s[i];
                      if(i == s.size()-1){
                        if(tmp == "TRUE")
                          gv_values[ind] = 1;
                       else gv_values[ind] = 0;
                       ind++; i++; tmp = "";
                      }
                  }
                  else if(s[i] == ','){//add to structures
                       if(tmp == "TRUE")
                          gv_values[ind] = 1;
                       else gv_values[ind] = 0;
                       ind++; i++; tmp = "";
                  }
                  
                }
             }
             else{
               if(s[0] == '-'){
                   std::string act="", prop="", form="";
                   int part = 0;
                   for(int i=2;i<s.size();i++){
                          if(part == 0 && s[i]!=' ') act+=s[i];   
                          else if(part == 0 && s[i] == ' ') part++;
                          else if(part == 1 && s[i]!=' ') prop+=s[i];
                          else if(part == 1 && s[i] == ' '){ part++; i+=2;}
                          else if(part == 2){
                              form+=s[i];
                          }
                   }

                   if (gammaMinus.find(act) != gammaMinus.end())
                       {
                          formula *f = new formula(form);
                          gammaMinus[act][prop] = f;
                     } 
                  else{
                    std::unordered_map<std::string,formula*> nm;
                    formula *f = new formula(form);
                    nm[prop] = f;
                    gammaMinus[act] = nm;
                  }
               }
               else if(s[0] == '+'){
                   std::string act="", prop="", form="";
                   int part = 0;
                   for(int i=2;i<s.size();i++){
                          if(part == 0 && s[i]!=' ') act+=s[i];   
                          else if(part == 0 && s[i] == ' ') part++;
                          else if(part == 1 && s[i]!=' ') prop+=s[i];
                          else if(part == 1 && s[i] == ' '){ part++; i+=2;}
                          else if(part == 2){
                              form+=s[i];
                          }
                   }

                   if (gammaPlus.find(act) != gammaPlus.end())
                       {
                          formula *f = new formula(form);
                          gammaPlus[act][prop] = f;
                     } 
                  else{
                    std::unordered_map<std::string,formula*> nm;
                    formula *f = new formula(form);
                    nm[prop] = f;
                    gammaPlus[act] = nm;
                  }
               }
               else if(s[0] == 'l' && s[2] == ':'){
                std::string st = "";
                for(int i=4;i<s.size();i++)
                          st+=s[i];
                formula *f = new formula(st);
                 local_formulas.push_back(f);
               }
               else if(s[0] == 'g' && s[2] == ':'){
                    std::string st = "";
                for(int i=4;i<s.size();i++)
                          st+=s[i];
                formula *f = new formula(st);
                 global_formulas.push_back(f);
               }
               else{//readActions
                    std::string act="";
                      for(int i=0;i<s.size();i++){
                        if(s[i]!=' ') act+=s[i];
                        else{
                          actions.push_back(act);
                          act="";
                        }
                      }
                      actions.push_back(act);
             }
           }
             
             line++;
         }


         //read actions ?
         //read gamma (+/-)
         //read formulae (l/g)
         //read the sequence of actions
}

int main(void){

  std::ifstream input("ProblemDescriptionC.txt");

 readProblem(input);
 input.close();

std::cout<<"Ispis ucitanih struktura: "<<std::endl<<std::endl;

std::unordered_map<std::string, int>::iterator it = lv_key.begin();

    while (it != lv_key.end()) {
        std::string lv = it->first;
        int kljuc = it->second;
        std::cout << lv << " :: " << kljuc << std::endl;
        it++;
    }

std::cout<<std::endl<<std::endl;

std::unordered_map<int, int>::iterator it1 = lKey_index.begin();

    while (it1 != lKey_index.end()) {
        int lk = it1->first;
        int indeks = it1->second;
        std::cout << lk << " :: " << indeks << std::endl;
        it1++;
    }

std::cout<<std::endl<<std::endl;


it = gv_key.begin();

    while (it != gv_key.end()) {
        std::string gv = it->first;
        int kljuc = it->second;
        std::cout << gv << " :: " << kljuc << std::endl;
        it++;
    }

std::cout<<std::endl<<std::endl;

it1 = gKey_index.begin();

    while (it1 != gKey_index.end()) {
        int gk = it1->first;
        int indeks = it1->second;
        std::cout << gk << " :: " << indeks << std::endl;
        it1++;
    }

std::cout<<std::endl<<std::endl;


it = operator_key.begin();

    while (it != operator_key.end()) {
        std::string op = it->first;
        int kljuc = it->second;
        std::cout << op << " :: " << kljuc << std::endl;
        it++;
    }

std::cout<<std::endl<<std::endl;

std::unordered_map<int, std::string>::iterator it2 = key_operator.begin();

    while (it2 != key_operator.end()) {
        int kljuc = it2->first;
        std::string op= it2->second;
        std::cout << kljuc << " :: " << op << std::endl;
        it2++;
    }

    std::cout<<std::endl<<std::endl;

    for(int i=0;i<lvSize;i++)
     std::cout<<lv_values[i]<<" ";

     std::cout<<std::endl<<std::endl;

    for(int i=0;i<gvSize;i++)
     std::cout<<gv_values[i]<<" ";

     std::cout<<std::endl<<std::endl;

     for(int i=0;i<actions.size();i++)
        std::cout<<actions[i]<<" ";

      std::cout<<std::endl<<std::endl; 
 
      std::cout<<"Local formulas: "<<std::endl;
      for(int i=0;i<local_formulas.size();i++){
         local_formulas[i]->print(0);
         std::cout<<std::endl;
      }

      std::cout<<"Global formulas: "<<std::endl;
      for(int i=0;i<global_formulas.size();i++){
         global_formulas[i]->print(0);
         std::cout<<std::endl;
      }

std::cout<<std::endl<<std::endl;
std::cout<<"Gamma minus: "<<std::endl;
for (std::unordered_map<std::string,std::unordered_map<std::string,formula*>>::const_iterator it = gammaMinus.begin(); it != gammaMinus.end(); ++it) {
                     std::string act = it->first;
                     std::unordered_map<std::string,formula*> tmp = it->second;
                     
                     for (std::unordered_map<std::string,formula*>::const_iterator it1 = tmp.begin(); it1 != tmp.end(); ++it1) {
                          std::string prop = it1->first;
                          formula* f = it1->second;
                          f->print(0);
                          std::cout<<std::endl;
                     }
                    
                 }


std::cout<<std::endl<<std::endl;
std::cout<<"Gamma plus: "<<std::endl;
for (std::unordered_map<std::string,std::unordered_map<std::string,formula*>>::const_iterator it = gammaPlus.begin(); it != gammaPlus.end(); ++it) {
                     std::string act = it->first;
                     std::unordered_map<std::string,formula*> tmp = it->second;
                     
                     for (std::unordered_map<std::string,formula*>::const_iterator it1 = tmp.begin(); it1 != tmp.end(); ++it1) {
                          std::string prop = it1->first;
                          formula* f = it1->second;
                          f->print(0);
                          std::cout<<std::endl;
                     }                
                 }

//execute actions in turn
//make changes with gammas where possible
//evaluate all local and global formulae

std::cout<<"Evaluacija lokalnih formula: "<<std::endl;
for(int i=0;i<local_formulas.size();i++)
     std::cout<<local_formulas[i]->evaluate()<<std::endl;

std::cout<<std::endl;
std::cout<<"Evaluacija globalnih formula: "<<std::endl;
for(int i=0;i<global_formulas.size();i++)
     std::cout<<global_formulas[i]->evaluate()<<std::endl;

for(int i=0;i<actions.size();i++){

  if(i == actions.size()-1){
    for(int k=0;k<local_formulas.size();k++)
            local_formulas[k]->setLastAction();
    for(int k=0;k<global_formulas.size();k++)
            global_formulas[k]->setLastAction();  
  }

  std::string ac = actions[i];
  std::unordered_map<std::string,formula*> tmpMinus = gammaMinus[ac];
   for (std::unordered_map<std::string,formula*>::const_iterator it1 = tmpMinus.begin(); it1 != tmpMinus.end(); ++it1) {//evaluate action effect on all props
               std::string prop = it1->first;
               formula* f = it1->second;
               int val = f->evaluate();

               if(val == 1){//change proposition value to 0
                   if(lv_key.find(prop) != lv_key.end()){//it is a local prop
                    int key = lv_key[prop];
                    int ind =  lKey_index[key];
                    lv_values[ind] = 0;
               }
               else if(gv_key.find(prop) != gv_key.end()){//it is a global prop
                     int key = gv_key[prop];
                    int ind =  gKey_index[key];
                    gv_values[ind] = 0;
               }
           }

   }

    std::unordered_map<std::string,formula*> tmpPlus = gammaPlus[ac];
   for (std::unordered_map<std::string,formula*>::const_iterator it1 = tmpPlus.begin(); it1 != tmpPlus.end(); ++it1) {//evaluate action effect on all props
               std::string prop = it1->first;
               formula* f = it1->second;
               int val = f->evaluate();

               if(val == 1){//change proposition value to 1
                   if(lv_key.find(prop) != lv_key.end()){//it is a local prop
                    int key = lv_key[prop];
                    int ind =  lKey_index[key];
                    lv_values[ind] = 1;
               }
               else if(gv_key.find(prop) != gv_key.end()){//it is a global prop
                     int key = gv_key[prop];
                    int ind =  gKey_index[key];
                    gv_values[ind] = 1;
               }
           }

   }

   /*std::cout<<"Action: "<<ac<<std::endl;
   int key = gv_key["congestion"];
           int index = gKey_index[key];
           std::cout<<"congestion: "<<gv_values[index]<<std::endl;

      key = gv_key["lowBattery"];
           index = gKey_index[key];
           std::cout<<"lowBattery: "<<gv_values[index]<<std::endl;


     key = lv_key["on6"];
           index = lKey_index[key];
           std::cout<<"on6: "<<lv_values[index]<<std::endl;

      key = lv_key["on4"];
           index = lKey_index[key];
           std::cout<<"on4: "<<lv_values[index]<<std::endl;

       key = lv_key["broken4"];
           index = lKey_index[key];
           std::cout<<"broken4: "<<lv_values[index]<<std::endl;*/

   //evaluate all formulas - quit if something is false
   for(int i=0;i<local_formulas.size();i++){
         int val = local_formulas[i]->evaluate();
         if(val == 0){ 
           std::cout<<"Lokalna formula je invalidirana!"<<std::endl;
           return -1;
         }
   }

   for(int i=0;i<global_formulas.size();i++){
         int val = global_formulas[i]->evaluate();
         if(val == 0){ 
           std::cout<<"Globalna formula je invalidirana!"<<std::endl;
           std::cout<<"Global variables: "<<std::endl;
           int key = gv_key["lowBattery"];
           int index = gKey_index[key];
           std::cout<<"low battery status: "<<gv_values[index]<<std::endl;
            for(int i=0;i<2;i++) std::cout<<gv_values[i]<<" "<<std::endl;
           return -1;
         }
   }

}

//std::vector<formula*> local_formulas;
//std::vector<formula*> global_formulas;



 delete []lv_values;
 delete []gv_values;

    return 0;
}