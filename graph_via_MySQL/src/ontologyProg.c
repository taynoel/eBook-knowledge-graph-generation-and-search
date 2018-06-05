#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <iostream>
#include "fact.h"
#include <vector>
#include <string>
#include <mysql.h>
#include "ontologyProg.h"

#define NUM_OF_ARGUMENTS 50
#define LENGTH_OF_STR 500
#define LENGTH_OF_STRs 100
#define LENGTH_OF_STRss 50

typedef struct node1{
    char str[LENGTH_OF_STR];
    bool b;
    int i;
    char c;
    long l;
}NTSTRCT_STR;

typedef struct node2{
    char subject[LENGTH_OF_STRss];
    char property[LENGTH_OF_STRss];
    char object[LENGTH_OF_STRss];
    char c;
}NTSTRCT_TRP;

char* NTf_readWholeTextFile(const char* FileName, long* BufferSize);
void NTf_ReasoningKernelCreate();
void NTf_ReasoningKernelRelease();
void NTf_addAxiom(NTSTRCT_STR* NTtriple);


fact_reasoning_kernel* KERNEL;

typedef struct node3{
    fact_axiom* axiom;
}NTSTRCT_axiom;

void NTf_ReasoningKernelCreate()
{
    if(KERNEL==NULL) KERNEL = fact_reasoning_kernel_new();
    else printf("Kernel already created!\n");
}
void NTf_ReasoningKernelRelease()
{
    fact_reasoning_kernel_free (KERNEL);
}



NTSTRCT_STR* NTf_Query(const char* QueryVariable,const char* QueryInstruction,bool *Error,int *NumResultFromQuery)
{// subject, predicate, object, subject, predicate, object...
//only supports: ?var,rdf:type,channel (a)
    //           ?var,nt:property,obj  (b)
    //           ?var,nt:property,?D   (c)
    //           subj,nt:property,?D   (2)
    //           ?var,nottype,chan    (d)//not used now
    char *buffer;
    char *PCH;
    char str[LENGTH_OF_STRs];
    NTSTRCT_TRP *NTtrp;
    NTSTRCT_STR* NTvar;
    NTSTRCT_axiom* NTaxm;
    int Count;
    buffer=(char*)malloc(sizeof(char)*(strlen(QueryInstruction)+1));
    strcpy(buffer, QueryInstruction);
    PCH=strtok(buffer, ",");
    Count=0;*Error=0;
    while (PCH!=NULL) {Count++;PCH=strtok(NULL, ",");}
    if((Count%3)!=0){*Error=1;return NULL;}
    int NumberRdf=Count/3;
    NTtrp=(NTSTRCT_TRP*)malloc(sizeof(NTSTRCT_TRP)*NumberRdf);
    NTvar=(NTSTRCT_STR*)malloc(sizeof(NTSTRCT_STR)*NumberRdf);
    strcpy(buffer, QueryInstruction);
    PCH=strtok(buffer, ",");
    Count=0;
    while (PCH!=NULL)
    {
        strcpy(NTtrp[Count].subject, PCH);PCH=strtok(NULL, ",");
        strcpy(NTtrp[Count].property, PCH);PCH=strtok(NULL, ",");
        strcpy(NTtrp[Count].object, PCH);PCH=strtok(NULL, ",");
        Count++;
    }
    int NumVariables=0;
    for(int f=0;f<NumberRdf;f++)
    {
        if(NTtrp[f].subject[0]=='?')
        {
            bool VariableFound=0;
            for(int f2=0;f2<NumVariables;f2++)
                if(!strcmp(NTtrp[f].subject, NTvar[f2].str)){VariableFound=1; break;}
            if(VariableFound==0){strcpy(NTvar[NumVariables].str, NTtrp[f].subject);NumVariables++;}
        }
        if(NTtrp[f].object[0]=='?')
        {
            bool VariableFound=0;
            VariableFound=0;
            for(int f2=0;f2<NumVariables;f2++)
                if(!strcmp(NTtrp[f].object, NTvar[f2].str)){VariableFound=1; break;}
            if(VariableFound==0){strcpy(NTvar[NumVariables].str, NTtrp[f].object);NumVariables++;}
        }
    }
    //NTvar=variable names with NumVariables, NTtrp=triple with NumRdf
    //calculate number variable inersection axiom
    int NumIntersectionAxiom=0;
    for(int f=0;f<NumVariables;f++)
        for(int f2=0;f2<NumberRdf;f2++)
            if(!strcmp(NTvar[f].str, NTtrp[f2].subject)){NumIntersectionAxiom++;break;}
    //check subject,property,?var
    int NumAxiom=NumIntersectionAxiom+NumberRdf;
    for(int f=0;f<NumberRdf;f++)
    {
        if(NTtrp[f].object[0]=='?')
        {
            if(NTtrp[f].subject[0]!='?'){NTtrp[f].c='2';NumAxiom++;}
            else NTtrp[f].c='c';
        }
        else if(NTtrp[f].subject[0]=='?' &&(!strcmp(NTtrp[f].property, "a") ||  !strcmp(NTtrp[f].property, "rdf:type")))
            NTtrp[f].c='a';
        else if (NTtrp[f].subject[0]=='?')
            NTtrp[f].c='b';
        else if (!strcmp(NTtrp[f].property,"nottype"))
            NTtrp[f].c='d';
        else {printf("Error106\n");exit(6);}
    }
    NTaxm=(NTSTRCT_axiom*)malloc(sizeof(NTSTRCT_axiom)*NumAxiom);
    Count=0;
    for(int f=0;f<NumberRdf;f++)
    {
        if(NTtrp[f].c=='a' || NTtrp[f].c=='d')
        {
            sprintf(str, "%s:%d",NTtrp[f].subject,f);
            fact_new_arg_list(KERNEL);
            fact_add_arg(KERNEL, (fact_expression*)fact_concept(KERNEL, str));
            fact_add_arg(KERNEL, (fact_expression*)fact_concept(KERNEL, NTtrp[f].object));
            NTaxm[Count].axiom=fact_equal_concepts(KERNEL);Count++;
        }
        else if(NTtrp[f].c=='b')
        {
            sprintf(str, "%s:%d",NTtrp[f].subject,f);
            fact_o_role_expression* prop=fact_object_role(KERNEL, NTtrp[f].property);
            fact_individual_expression* indv=fact_individual(KERNEL, NTtrp[f].object);
            fact_concept_expression* some = fact_o_value ( KERNEL, prop, indv);
            fact_new_arg_list(KERNEL);
            fact_add_arg(KERNEL, (fact_expression*)fact_concept(KERNEL, str));
            fact_add_arg(KERNEL, (fact_expression*)some);
            NTaxm[Count].axiom=fact_equal_concepts(KERNEL);Count++;
        }
        else if(NTtrp[f].c=='c')
        {
            sprintf(str, "%s:%d",NTtrp[f].subject,f);
            fact_o_role_expression* prop=fact_object_role(KERNEL, NTtrp[f].property);
            fact_concept_expression* clas=fact_concept(KERNEL, NTtrp[f].object);
            fact_concept_expression* some = fact_o_exists ( KERNEL, prop, clas);
            fact_new_arg_list(KERNEL);
            fact_add_arg(KERNEL, (fact_expression*)fact_concept(KERNEL, str));
            fact_add_arg(KERNEL, (fact_expression*)some);
            NTaxm[Count].axiom=fact_equal_concepts(KERNEL);Count++;
        }
        else if(NTtrp[f].c=='2')
        {
            sprintf(str, "%s:inverse",NTtrp[f].property);
            fact_o_role_expression* a=fact_object_role(KERNEL, NTtrp[f].property);
            fact_o_role_expression* ainv=fact_object_role(KERNEL, str);
            NTaxm[Count].axiom=fact_set_inverse_roles(KERNEL, ainv, a);Count++;
            fact_concept_expression* some = fact_o_value ( KERNEL, ainv, fact_individual(KERNEL, NTtrp[f].subject));
            sprintf(str, "%s:%d",NTtrp[f].object,f);
            fact_new_arg_list(KERNEL);
            fact_add_arg(KERNEL, (fact_expression*)fact_concept(KERNEL, str));
            fact_add_arg(KERNEL, (fact_expression*)some);
            NTaxm[Count].axiom=fact_equal_concepts(KERNEL);Count++;
        }
    }
    for(int f=0;f<NumVariables;f++)
    {
        fact_new_arg_list(KERNEL);
        for(int f2=0;f2<NumberRdf;f2++)
        {
            if(NTtrp[f2].c=='a' || NTtrp[f2].c=='b' || NTtrp[f2].c=='c')
            {
                if(!strcmp(NTvar[f].str, NTtrp[f2].subject))
                {
                    sprintf(str, "%s:%d",NTtrp[f2].subject,f2);
                    fact_add_arg(KERNEL, (fact_expression*)fact_concept(KERNEL, str));
                }
            }
            else if(NTtrp[f2].c=='2')
            {
                if(!strcmp(NTvar[f].str, NTtrp[f2].object))
                {
                    sprintf(str, "%s:%d",NTtrp[f2].object,f2);
                    fact_add_arg(KERNEL, (fact_expression*)fact_concept(KERNEL, str));
                }
            }
            else{printf("ERROR107!\n");exit(7);};
        }
        fact_concept_expression* some=fact_and(KERNEL);
        fact_new_arg_list(KERNEL);
        fact_add_arg(KERNEL, (fact_expression*)fact_concept(KERNEL, NTvar[f].str));
        fact_add_arg(KERNEL, (fact_expression*)some);
        NTaxm[Count].axiom=fact_equal_concepts(KERNEL);Count++;
    }
    fact_actor* actor = fact_individual_actor_new();
    fact_get_instances (KERNEL, fact_concept(KERNEL, QueryVariable),&actor);
    const char** Names= fact_get_elements_1d(actor);
    Count=0;
    while (Names[Count]!=NULL)Count++;
    int NumQueryResult=Count;
    NTSTRCT_STR* NTQueryAns=(NTSTRCT_STR*)malloc(sizeof(NTSTRCT_STR)*NumQueryResult);
    for(int f=0;f<NumQueryResult;f++) strcpy(NTQueryAns[f].str, Names[f]);
    
    //clear axioms
    for(int f=0;f<NumAxiom;f++)fact_retract(KERNEL, NTaxm[f].axiom);
    
    fact_actor_free(actor);free(buffer);free(NTaxm);free(NTvar);free(NTtrp);
    //printf("%d %d %d,\n",fact_is_instance (KERNEL,fact_individual(KERNEL, "vd"),fact_concept(KERNEL, "?a")),Count,NumAxiom);
    *NumResultFromQuery=NumQueryResult;
    return NTQueryAns;
}





void NTf_AddKBfromFile(const char* Filename)
{//add individual device KB, and create another ontology file for closed world assumption inference
    char *buffer,*buffer2;
    char str[LENGTH_OF_STR];
    long s;
    int count;
    NTSTRCT_STR *NTvar,*NTtriple;
    NTtriple=(NTSTRCT_STR*)malloc(sizeof(NTSTRCT_STR)*NUM_OF_ARGUMENTS);
    //subject predicate object //newclass pred property class
    buffer=NTf_readWholeTextFile(Filename, &s);
    buffer2=(char*)malloc(sizeof(char)*(strlen(buffer)+1));
    strcpy(buffer2, buffer);
    char* PCH=strtok(buffer2, ";\n");
    count=0;
    while(PCH!=NULL)
    {
        count++;
        PCH=strtok(NULL, ";\n");
    }
    int NUMofRDF=count;
    NTvar=(NTSTRCT_STR*)malloc(sizeof(NTSTRCT_STR)*(count));
    strcpy(buffer2, buffer);
    PCH=strtok(buffer2, ";\n");
    count=0;
    while(PCH!=NULL)
    {
        strcpy(NTvar[count].str, PCH);count++;
        PCH=strtok(NULL, ";\n");
    }
    
    
    for(int f=0;f<NUMofRDF;f++)
    {
        strcpy(str, NTvar[f].str);
        PCH=strtok(str, ",");
        count=0;
        while(PCH!=NULL)
        {
            strcpy(NTtriple[count].str, PCH);count++;
            PCH=strtok(NULL, ",");
        }
        NTf_addAxiom(NTtriple);
    }
    free(NTvar);free(NTtriple);free(buffer);free(buffer2);
}



void NTf_addAxiom(NTSTRCT_STR* NTtriple)
{
    if(!strcmp(NTtriple[0].str, "rdf:type"))
    {
        fact_individual_expression* a=fact_individual(KERNEL,NTtriple[1].str);
        fact_concept_expression* b=fact_concept(KERNEL, NTtriple[2].str);
        fact_instance_of(KERNEL, a, b);
    }
    else if(!strcmp(NTtriple[0].str, "rdfs:subclassof"))
    {
        fact_concept_expression* a=fact_concept(KERNEL, NTtriple[1].str);
        fact_concept_expression* b=fact_concept(KERNEL, NTtriple[2].str);
        fact_implies_concepts(KERNEL, a, b);
    }
    else if(!strcmp(NTtriple[0].str, "rdfs:subpropertyof"))
    {
        fact_o_role_expression* a=fact_object_role(KERNEL, NTtriple[1].str);
        fact_o_role_expression* b=fact_object_role(KERNEL, NTtriple[2].str);
        fact_implies_o_roles(KERNEL, (fact_o_role_complex_expression*)a, b);
    }
    else if(!strcmp(NTtriple[0].str, "DL:complexroleinclusion"))
    {//new role,number of role,n3.n2.n1   ()
        fact_o_role_expression* newRole=fact_object_role(KERNEL, NTtriple[1].str);
        fact_new_arg_list(KERNEL);
        int NumClass=atoi(NTtriple[2].str);
        for(int ff=0;ff<NumClass;ff++)
            fact_add_arg(KERNEL, (fact_expression*)fact_object_role(KERNEL, NTtriple[ff+3].str));
        fact_o_role_complex_expression* Complex=fact_compose(KERNEL);
        fact_implies_o_roles(KERNEL, Complex, newRole);
    }
    else if(!strcmp(NTtriple[0].str, "rdfs:domain"))
    {//property, class of domain
        fact_o_role_expression* a=fact_object_role(KERNEL, NTtriple[1].str);
        fact_concept_expression* b=fact_concept(KERNEL, NTtriple[2].str);
        fact_set_o_domain(KERNEL, a, b);
    }
    else if(!strcmp(NTtriple[0].str, "rdfs:range"))
    {//property, class of range
        fact_o_role_expression* a=fact_object_role(KERNEL, NTtriple[1].str);
        fact_concept_expression* b=fact_concept(KERNEL, NTtriple[2].str);
        fact_set_o_range(KERNEL, a, b);
    }
    else if(!strcmp(NTtriple[0].str, "owl:inverseof"))
    {
        fact_o_role_expression* a=fact_object_role(KERNEL, NTtriple[1].str);
        fact_o_role_expression* b=fact_object_role(KERNEL, NTtriple[2].str);
        fact_set_inverse_roles(KERNEL, a, b);
    }
    else if(!strcmp(NTtriple[0].str, "owl:symmetricproperty"))
    {
        fact_o_role_expression* a=fact_object_role(KERNEL, NTtriple[1].str);
        fact_set_symmetric(KERNEL, a);
    }
    else if(!strcmp(NTtriple[0].str, "owl:transitiveproperty"))
    {
        fact_o_role_expression* a=fact_object_role(KERNEL, NTtriple[1].str);
        fact_set_transitive(KERNEL, a);
    }
    else if(!strcmp(NTtriple[0].str, "owl:functionalproperty"))
    {
        fact_o_role_expression* a=fact_object_role(KERNEL, NTtriple[1].str);
        fact_set_o_functional(KERNEL, a);
    }
    else if(!strcmp(NTtriple[0].str, "owl:inversefunctionalproperty"))
    {
        fact_o_role_expression* a=fact_object_role(KERNEL, NTtriple[1].str);
        fact_set_inverse_functional(KERNEL, a);
    }
    else if(!strcmp(NTtriple[0].str, "owl:sameaspair"))
    {
        fact_individual_expression* a=fact_individual(KERNEL,NTtriple[1].str);
        fact_individual_expression* b=fact_individual(KERNEL,NTtriple[2].str);
        fact_new_arg_list(KERNEL);
        fact_add_arg(KERNEL, (fact_expression*)a);
        fact_add_arg(KERNEL, (fact_expression*)b);
        fact_process_same(KERNEL);
    }
    else if(!strcmp(NTtriple[0].str, "owl:differentfrom"))
    {//num of ind,...individuals...
        int NumClass=atoi(NTtriple[1].str);
        fact_new_arg_list(KERNEL);
        for(int ff=0;ff<NumClass;ff++)
            fact_add_arg(KERNEL, (fact_expression*)fact_individual(KERNEL, NTtriple[ff+2].str));
        fact_process_different(KERNEL);
    }
    else if(!strcmp(NTtriple[0].str, "owl:equivalentclasspair"))
    {
        fact_concept_expression* a=fact_concept(KERNEL, NTtriple[1].str);
        fact_concept_expression* b=fact_concept(KERNEL, NTtriple[2].str);
        fact_new_arg_list(KERNEL);
        fact_add_arg(KERNEL, (fact_expression*)a);
        fact_add_arg(KERNEL, (fact_expression*)b);
        fact_equal_concepts(KERNEL);
    }
    else if(!strcmp(NTtriple[0].str, "owl:equivalentpropertypair"))
    {
        fact_o_role_expression* a=fact_object_role(KERNEL, NTtriple[1].str);
        fact_o_role_expression* b=fact_object_role(KERNEL, NTtriple[2].str);
        fact_new_arg_list(KERNEL);
        fact_add_arg(KERNEL, (fact_expression*)a);
        fact_add_arg(KERNEL, (fact_expression*)b);
        fact_equal_o_roles(KERNEL);
    }
    else if(!strcmp(NTtriple[0].str, "owl:somevaluesfrom"))
    {//newclass, property, old class
        fact_concept_expression* newclass=fact_concept(KERNEL, NTtriple[1].str);
        fact_o_role_expression* prop=fact_object_role(KERNEL, NTtriple[2].str);
        fact_concept_expression* clas=fact_concept(KERNEL, NTtriple[3].str);
        fact_concept_expression* some = fact_o_exists ( KERNEL, prop, clas);
        fact_new_arg_list(KERNEL);
        fact_add_arg(KERNEL, (fact_expression*)newclass);
        fact_add_arg(KERNEL, (fact_expression*)some);
        fact_equal_concepts(KERNEL);
    }
    else if(!strcmp(NTtriple[0].str, "owl:allvaluesfrom"))
    {//newclass, property, old class
        fact_concept_expression* newclass=fact_concept(KERNEL, NTtriple[1].str);
        fact_o_role_expression* prop=fact_object_role(KERNEL, NTtriple[2].str);
        fact_concept_expression* clas=fact_concept(KERNEL, NTtriple[3].str);
        fact_concept_expression* some = fact_o_forall ( KERNEL, prop, clas);
        fact_new_arg_list(KERNEL);
        fact_add_arg(KERNEL, (fact_expression*)newclass);
        fact_add_arg(KERNEL, (fact_expression*)some);
        fact_equal_concepts(KERNEL);
    }
    else if(!strcmp(NTtriple[0].str, "owl:hasvalue"))
    {//newclass, property, old class
        fact_concept_expression* newclass=fact_concept(KERNEL, NTtriple[1].str);
        fact_o_role_expression* prop=fact_object_role(KERNEL, NTtriple[2].str);
        fact_individual_expression* indv=fact_individual(KERNEL, NTtriple[3].str);
        fact_concept_expression* some = fact_o_value ( KERNEL, prop, indv);
        fact_new_arg_list(KERNEL);
        fact_add_arg(KERNEL, (fact_expression*)newclass);
        fact_add_arg(KERNEL, (fact_expression*)some);
        fact_equal_concepts(KERNEL);
    }
    else if(!strcmp(NTtriple[0].str, "owl:unionof"))
    {//newclass,num of class,...classes...
        int NumClass=atoi(NTtriple[2].str);
        fact_new_arg_list(KERNEL);
        for(int ff=0;ff<NumClass;ff++)
            fact_add_arg(KERNEL, (fact_expression*)fact_concept(KERNEL, NTtriple[ff+3].str));
        fact_concept_expression* some=fact_or(KERNEL);
        fact_new_arg_list(KERNEL);
        fact_add_arg(KERNEL, (fact_expression*)fact_concept(KERNEL, NTtriple[1].str));
        fact_add_arg(KERNEL, (fact_expression*)some);
        fact_equal_concepts(KERNEL);
    }
    else if(!strcmp(NTtriple[0].str, "owl:intersectionof"))
    {//newclass,num of class,...classes...
        int NumClass=atoi(NTtriple[2].str);
        fact_new_arg_list(KERNEL);
        for(int ff=0;ff<NumClass;ff++)
            fact_add_arg(KERNEL, (fact_expression*)fact_concept(KERNEL, NTtriple[ff+3].str));
        fact_concept_expression* some=fact_and(KERNEL);
        fact_new_arg_list(KERNEL);
        fact_add_arg(KERNEL, (fact_expression*)fact_concept(KERNEL, NTtriple[1].str));
        fact_add_arg(KERNEL, (fact_expression*)some);
        fact_equal_concepts(KERNEL);
    }
    else if(!strcmp(NTtriple[0].str, "owl:complementof"))
    {//new complement class, old class
        
        fact_concept_expression* some=fact_not(KERNEL, fact_concept(KERNEL, NTtriple[2].str));
        fact_new_arg_list(KERNEL);
        fact_add_arg(KERNEL, (fact_expression*)fact_concept(KERNEL, NTtriple[1].str));
        fact_add_arg(KERNEL, (fact_expression*)some);
        fact_equal_concepts(KERNEL);
    }
    else if(!strcmp(NTtriple[0].str, "owl:disjointwith"))
    {//num of class,...classes...
        int NumClass=atoi(NTtriple[1].str);
        fact_new_arg_list(KERNEL);
        for(int ff=0;ff<NumClass;ff++)
            fact_add_arg(KERNEL, (fact_expression*)fact_concept(KERNEL, NTtriple[ff+2].str));
        fact_disjoint_concepts(KERNEL);
    }
    else if(!strcmp(NTtriple[0].str, "owl:disjointunion"))
    {//new class,num of class,...classes...
        int NumClass=atoi(NTtriple[2].str);
        fact_new_arg_list(KERNEL);
        for(int ff=0;ff<NumClass;ff++)
            fact_add_arg(KERNEL, (fact_expression*)fact_concept(KERNEL, NTtriple[ff+3].str));
        fact_disjoint_union(KERNEL, fact_concept(KERNEL, NTtriple[1].str));
    }
    else if(!strcmp(NTtriple[0].str, "owl:oneof"))
    {//newclass,num of individual,...individuals...
        int NumClass=atoi(NTtriple[2].str);
        fact_new_arg_list(KERNEL);
        for(int ff=0;ff<NumClass;ff++)
            fact_add_arg(KERNEL, (fact_expression*)fact_individual(KERNEL, NTtriple[ff+3].str));
        fact_concept_expression* some=fact_one_of(KERNEL);
        fact_new_arg_list(KERNEL);
        fact_add_arg(KERNEL, (fact_expression*)fact_concept(KERNEL, NTtriple[1].str));
        fact_add_arg(KERNEL, (fact_expression*)some);
        fact_equal_concepts(KERNEL);
    }
    else if(!strcmp(NTtriple[0].str, "owl:cardinalitytop"))
    {//newclass,property,cardinaltiy
        fact_o_role_expression* prop=fact_object_role(KERNEL, NTtriple[2].str);
        fact_concept_expression* newclas=fact_concept(KERNEL, NTtriple[1].str);
        fact_concept_expression* some=fact_o_cardinality(KERNEL, atoi(NTtriple[3].str), prop, fact_top(KERNEL));
        fact_new_arg_list(KERNEL);
        fact_add_arg(KERNEL, (fact_expression*)newclas);
        fact_add_arg(KERNEL, (fact_expression*)some);
        fact_equal_concepts(KERNEL);
        
    }
    else if(!strcmp(NTtriple[0].str, "owl:cardinality"))
    {//newclass,property,cardinaltiy,intendedClass
        fact_o_role_expression* prop=fact_object_role(KERNEL, NTtriple[2].str);
        fact_concept_expression* newclas=fact_concept(KERNEL, NTtriple[1].str);
        fact_concept_expression* intendedclass=fact_concept(KERNEL, NTtriple[4].str);
        fact_concept_expression* some=fact_o_cardinality(KERNEL, atoi(NTtriple[3].str), prop, intendedclass);
        fact_new_arg_list(KERNEL);
        fact_add_arg(KERNEL, (fact_expression*)newclas);
        fact_add_arg(KERNEL, (fact_expression*)some);
        fact_equal_concepts(KERNEL);
        
    }
    else if(!strcmp(NTtriple[0].str, "owl:cardinalitytopbn"))
    {//individual,property,cardinaltiy
        fact_o_role_expression* prop=fact_object_role(KERNEL, NTtriple[2].str);
        fact_individual_expression* ind=fact_individual(KERNEL, NTtriple[1].str);
        fact_concept_expression* some=fact_o_cardinality(KERNEL, atoi(NTtriple[3].str), prop, fact_top(KERNEL));
        fact_instance_of(KERNEL, ind, some);
    }
    else if(!strcmp(NTtriple[0].str, "owl:maxcardinalitytop"))
    {//newclass,property,cardinaltiy
        fact_o_role_expression* prop=fact_object_role(KERNEL, NTtriple[2].str);
        fact_concept_expression* newclas=fact_concept(KERNEL, NTtriple[1].str);
        fact_concept_expression* some=fact_o_max_cardinality(KERNEL, atoi(NTtriple[3].str), prop, fact_top(KERNEL));
        fact_new_arg_list(KERNEL);
        fact_add_arg(KERNEL, (fact_expression*)newclas);
        fact_add_arg(KERNEL, (fact_expression*)some);
        fact_equal_concepts(KERNEL);
    }
    else if(!strcmp(NTtriple[0].str, "owl:mincardinalitytop"))
    {//newclass,property,cardinaltiy
        fact_o_role_expression* prop=fact_object_role(KERNEL, NTtriple[2].str);
        fact_concept_expression* newclas=fact_concept(KERNEL, NTtriple[1].str);
        fact_concept_expression* some=fact_o_min_cardinality(KERNEL, atoi(NTtriple[3].str), prop, fact_top(KERNEL));
        fact_new_arg_list(KERNEL);
        fact_add_arg(KERNEL, (fact_expression*)newclas);
        fact_add_arg(KERNEL, (fact_expression*)some);
        fact_equal_concepts(KERNEL);
    }
    else if(!strcmp(NTtriple[0].str, "owl2:relatedtovaluestr"))
    {//individual data_role string
        fact_individual_expression* a=fact_individual(KERNEL, NTtriple[1].str);
        fact_d_role_expression* b=fact_data_role(KERNEL, NTtriple[2].str);
        fact_data_value_expression* c=fact_data_value(KERNEL, NTtriple[3].str, fact_get_str_data_type(KERNEL));
        fact_value_of(KERNEL, a, b, c);
    }
    else if(!strcmp(NTtriple[0].str, "nt:CSP"))
    {
        
    }
    else if(!strcmp(NTtriple[0].str, ":query"))
    {
        //Command for query. do nothing
    }
    else if(!strcmp(NTtriple[0].str, "//"))
    {
        //Remark. do nothing
    }
    //        else if(!strcmp(NTtriple[0].str, "snt:hasservice"))
    //        {
    //            fact_individual_expression* a=fact_individual(KERNEL, NTtriple[1].str);
    //            fact_o_role_expression* prop=fact_object_role(KERNEL, NTtriple[0].str);
    //            fact_individual_expression* c=fact_individual(KERNEL, NTtriple[2].str);
    //            fact_related_to(KERNEL, a, prop, c);
    //        }
    else
    {
        if(NTtriple[0].str[0]!='n' && NTtriple[0].str[0]!='t'){printf("Syntax error\n");exit(-1);}
        //printf("Normal RDF\n");
        fact_individual_expression* a=fact_individual(KERNEL, NTtriple[1].str);
        fact_o_role_expression* prop=fact_object_role(KERNEL, NTtriple[0].str);
        fact_individual_expression* c=fact_individual(KERNEL, NTtriple[2].str);
        fact_related_to(KERNEL, a, prop, c);
    }
}










char* NTf_readWholeTextFile(const char* FileName, long* BufferSize)
{
    char *buffer;
    long test;
    FILE *fh = fopen(FileName, "rb");
    if ( fh != NULL )
    {
        fseek(fh, 0L, SEEK_END);
        *BufferSize = ftell(fh);
        rewind(fh);
        buffer = (char*)malloc(sizeof(char)*(*BufferSize+1));
        if ( buffer != NULL )
        {
            test=fread(buffer, *BufferSize, 1, fh);
            fclose(fh); fh = NULL;
        }
        if (fh != NULL) fclose(fh);
    }
    buffer[*BufferSize]='\0';
    return buffer;
}

int NTapp ( int MODEE )
{
	//int MODEE=2;
	
	if(MODEE==0)
	{
		NTf_ReasoningKernelCreate();
		printf("////////////////////////////////////////////////////////////////////\n");
		for(int f=0;f<10;f++)printf("\n");
		printf("               Co-Learning Syllabus Query                            \n");
		for(int f=0;f<10;f++)printf("\n");
		printf("////////////////////////////////////////////////////////////////////\n");
	
		NTf_AddKBfromFile("/home/noel/CProg/EBook_KWS/FCS-ebook/BookOntology/FoundationsofComputerScience_Concept.txt");
		NTf_AddKBfromFile("/home/noel/CProg/EBook_KWS/FCS-ebook/BookOntology/FoundationsofComputerScience_Term.txt");
		NTf_AddKBfromFile("/home/noel/CProg/EBook_KWS/FCS-ebook/BookOntology/FoundationsofComputerScience_Explanation.txt");
		NTf_AddKBfromFile("/home/noel/CProg/EBook_KWS/FCS-ebook/BookOntology/FoundationsofComputerScienceOntology_c1.txt");
		NTf_AddKBfromFile("/home/noel/CProg/EBook_KWS/FCS-ebook/BookOntology/FoundationsofComputerScienceOntology_c2.txt");
		NTf_AddKBfromFile("/home/noel/CProg/EBook_KWS/FCS-ebook/BookOntology/FoundationsofComputerScienceOntology_c3.txt");
		NTf_AddKBfromFile("/home/noel/CProg/EBook_KWS/FCS-ebook/BookOntology/FoundationsofComputerScienceOntology_c7.txt");
		bool K;
    		int NumQueryRes;
    		//NTSTRCT_STR* AA= NTf_Query("?a", "?a,a,classfixed", &K,&NumQueryRes);
    		//NTSTRCT_STR* AA= NTf_Query("?a", "?a,a,computerSystemSoftware", &K,&NumQueryRes);
    		//NTSTRCT_STR* AA= NTf_Query("?a", "?a,nt:isQuestionFor,?b,?b,a,memoryManagers,?b,a,operatingSystem", &K,&NumQueryRes);

Loop:
    		printf("\n\nInput Query:\n");
    		char queryvarstr[10];
    		char querystr[200];
    		scanf("%s",queryvarstr);
    		scanf("%s",querystr);
    		printf("\n\nResults:\n==========\n");
    		NTSTRCT_STR* AA= NTf_Query(queryvarstr, querystr, &K,&NumQueryRes);
    	
    		for(int f=0;f<NumQueryRes;f++)
    		{
        		printf("%s %d\n",AA[f].str,NumQueryRes);
    		}
    		goto Loop;
	}
	if(MODEE==1)
	{
		NTf_ReasoningKernelCreate();
		printf("////////////////////////////////////////////////////////////////////\n");
		for(int f=0;f<10;f++)printf("\n");
		printf("               Co-Learning Syllabus Query                            \n");
		for(int f=0;f<10;f++)printf("\n");
		printf("////////////////////////////////////////////////////////////////////\n");
	
		NTf_AddKBfromFile("/home/noel/CProg/EBook_KWS/FCS-ebook/BookOntology/wConcept/FoundationsofComputerScience_Concept.txt");
		NTf_AddKBfromFile("/home/noel/CProg/EBook_KWS/FCS-ebook/BookOntology/wConcept/FoundationsofComputerScience_Term.txt");
		NTf_AddKBfromFile("/home/noel/CProg/EBook_KWS/FCS-ebook/BookOntology/wConcept/FoundationsofComputerScience_Explanation.txt");
		NTf_AddKBfromFile("/home/noel/CProg/EBook_KWS/FCS-ebook/BookOntology/wConcept/FoundationsofComputerScienceOntology_c1.txt");
		NTf_AddKBfromFile("/home/noel/CProg/EBook_KWS/FCS-ebook/BookOntology/wConcept/FoundationsofComputerScienceOntology_c2.txt");
		NTf_AddKBfromFile("/home/noel/CProg/EBook_KWS/FCS-ebook/BookOntology/wConcept/FoundationsofComputerScienceOntology_c3.txt");
		NTf_AddKBfromFile("/home/noel/CProg/EBook_KWS/FCS-ebook/BookOntology/wConcept/FoundationsofComputerScienceOntology_c7.txt");
		bool K;
    		int NumQueryRes;
    		//NTSTRCT_STR* AA= NTf_Query("?a", "?a,a,classfixed", &K,&NumQueryRes);
    		//NTSTRCT_STR* AA= NTf_Query("?a", "?a,a,computerSystemSoftware", &K,&NumQueryRes);
    		//NTSTRCT_STR* AA= NTf_Query("?a", "?a,nt:isQuestionFor,?b,?b,a,memoryManagers,?b,a,operatingSystem", &K,&NumQueryRes);
    		NTSTRCT_STR* AA= NTf_Query("?a", "?a,nt:nextPage,o:descp:storingData", &K,&NumQueryRes);
    	
    		for(int f=0;f<NumQueryRes;f++)
    		{
        		printf("%s %d\n",AA[f].str,NumQueryRes);
    		}
    		
	}
	else if(MODEE==2)//Generate all triples and store them in MySQL
	{
		std::vector<NTSTRCT_TRP> ListTriples;
		int numTriples=0;
		std::vector<std::string> ListRoles;
		int numRoles=0;
		std::vector<std::string> ListInst;
		int numInst=0;
		std::vector<std::string> ListConcept;
		int numConcept=0;
		
		NTf_ReasoningKernelCreate();
		NTf_AddKBfromFile("./BookOntology/dummyData.txt");
		
		
		//Get Role List
		fact_o_role_expression* o_top = fact_object_role_top(KERNEL);
		fact_actor* actor = fact_o_role_actor_new();
		fact_get_sub_roles(KERNEL,(fact_role_expression*)o_top,0,&actor);
		const char** Names= fact_get_elements_1d(actor);
		int Count=0;
    		while (Names[Count]!=NULL)Count++;
    		for(int f=0;f<Count;f++) 
    		{
    			printf("%s\n",Names[f]);
    			if(Names[f][0]!='-' && strcmp(Names[f],"*EROLE*"))
    			{
    				ListRoles.push_back(Names[f]);
    				numRoles++;
				}
			}
    		fact_actor_free(actor);
    		
    	//Get instance List
    	fact_concept_expression* c_top=fact_top(KERNEL);
		actor = fact_individual_actor_new();
		fact_get_instances(KERNEL,c_top,&actor);
		Names= fact_get_elements_1d(actor);
		Count=0;
    		while (Names[Count]!=NULL)Count++;
    		numInst=Count;
    		for(int f=0;f<numInst;f++) 
    		{
    			//#Note: Print Instance: printf("Instance%s\n",Names[f]);
    			ListInst.push_back(Names[f]);
    		}
		fact_actor_free(actor);
		
		
		//Get concept List
		actor = fact_concept_actor_new();
		fact_get_sub_concepts(KERNEL,c_top,0,&actor);
		Names= fact_get_elements_1d(actor);
		Count=0;
    		while (Names[Count]!=NULL)Count++;
    		for(int f=0;f<Count;f++) 
    		{
    			//#Note: Concept Print: printf("Concept: %s\n",Names[f]);
    			
    			if(strcmp(Names[f],"BOTTOM"))
    			{
    				ListConcept.push_back(Names[f]);
    				numConcept++;
				}
    		}
		fact_actor_free(actor);
		
		//Get inst to inst relation triple
		bool K;
		int NumQueryRes;
		char buffer[LENGTH_OF_STR];
		for(int f=0;f<numInst;f++)
		{
			if(f%100==0)
			    printf("Instance %d of %d\n",f+1,numInst);
			for(int f2=0;f2<numRoles;f2++)
			{
				sprintf(buffer,"%s,%s,?a",ListInst[f].c_str(),ListRoles[f2].c_str());
				NTSTRCT_STR* AA= NTf_Query("?a", buffer,&K,&NumQueryRes);
				if(NumQueryRes!=0)
				{
					NTSTRCT_TRP some;
					sprintf(some.subject,"%s",ListInst[f].c_str());
					sprintf(some.property,"%s",ListRoles[f2].c_str());
					for(int f3=0;f3<NumQueryRes;f3++)
					{
						 sprintf(some.object,"%s",AA[f3].str);
						 ListTriples.push_back(some);
						 numTriples++;
						 //printf("%s,%s,%s\n",ListTriples[numTriples-1].subject,ListTriples[numTriples-1].property,ListTriples[numTriples-1].object);
					}
				}
			}
		}
		
		//Get concept and inst relation triple
		for(int f=0;f<numConcept;f++)
		{
			strcpy(buffer,ListConcept[f].c_str());
			fact_concept_expression* some=fact_concept(KERNEL, buffer);
			actor = fact_individual_actor_new();
			
			fact_get_instances (KERNEL, some,&actor);
			//fact_get_direct_instances(KERNEL,c_top,&actor);
			
			Names= fact_get_elements_1d(actor);
			Count=0;
    		while (Names[Count]!=NULL)Count++;
    		NTSTRCT_TRP some2;
			sprintf(some2.object,"%s",buffer);
			sprintf(some2.property,"rdf:type");
    		for(int f2=0;f2<Count;f2++) 
    		{
				sprintf(some2.subject,"%s",Names[f2]);
    			ListTriples.push_back(some2);
    			numTriples++;
    		}
			fact_actor_free(actor);
		}
		
		//Get concept and concept relation triple
		for(int f=0;f<numConcept;f++)
		{
			strcpy(buffer,ListConcept[f].c_str());
			fact_concept_expression* some=fact_concept(KERNEL, buffer);
			actor = fact_concept_actor_new();
			fact_get_instances (KERNEL, some,&actor);
			fact_get_sub_concepts (KERNEL, some,1, &actor);
			Names= fact_get_elements_1d(actor);
			Count=0;
    		while (Names[Count]!=NULL)Count++;
    		NTSTRCT_TRP some2;
			sprintf(some2.object,"%s",buffer);
			sprintf(some2.property,"rdfs:subclassof");
    		for(int f2=0;f2<Count;f2++) 
    		{
				if(strcmp(Names[f2],"BOTTOM"))
				{
					sprintf(some2.subject,"%s",Names[f2]);
					ListTriples.push_back(some2);
					numTriples++;
				}
    		}
			fact_actor_free(actor);
		}
		
		
		MYSQL *con=mysql_init(NULL);
	    if(con==NULL)exit(-1);
	    if(mysql_real_connect(con,"localhost","monty","some_pass","robot",0,NULL,0)==NULL)exit(-2);
	    
	    mysql_query(con,"TRUNCATE FCSbookTripleW");
	    mysql_query(con,"TRUNCATE FCSbookTripleInstListW");
	    mysql_query(con,"TRUNCATE FCSbookTripleRoleListW");
	    
	    std::string stdString;
	    stdString.append("INSERT INTO FCSbookTripleW (subject,property,object,weight) VALUES");
		for(int f=0;f<numTriples;f++)
		{
		   //#Note: Triple print: printf("%d: %s,%s,%s,%f\n",f,ListTriples[f].subject,ListTriples[f].property,ListTriples[f].object,1.0);
		   if(f==(numTriples-1))
				sprintf(buffer,"('%s','%s','%s',%f)",ListTriples[f].subject,ListTriples[f].property,ListTriples[f].object,1.0);
		   else
				sprintf(buffer,"('%s','%s','%s',%f),",ListTriples[f].subject,ListTriples[f].property,ListTriples[f].object,1.0);
		   stdString.append(buffer);
	    }
	    mysql_query(con,stdString.c_str());
		
		stdString.clear();
		stdString.append("INSERT INTO FCSbookTripleInstListW (instance) VALUES");
		for(int f=0;f<numInst;f++)
		{
			sprintf(buffer,"('%s'),",ListInst[f].c_str());
			stdString.append(buffer);
		}
		for(int f=0;f<numConcept;f++)
		{
			sprintf(buffer,"('%s'),",ListConcept[f].c_str());
			stdString.append(buffer);
		}	
		stdString.erase(stdString.size()-1);
		mysql_query(con,stdString.c_str());
		
		stdString.clear();
		stdString.append("INSERT INTO FCSbookTripleRoleListW (role) VALUES");
		stdString.append("('rdf:type'),");
		stdString.append("('rdfs:subclassof'),");
		for(int f=0;f<numRoles;f++)
		{
			sprintf(buffer,"('%s'),",ListRoles[f].c_str());
			stdString.append(buffer);
		}
		stdString.erase(stdString.size()-1);
		mysql_query(con,stdString.c_str());
		
		
		
		printf("Roles:%d Concepts:%d Instances:%d Triples:%d \n",numRoles,numConcept,numInst,numTriples);
		
		
		
	}
	
	return 0;
	//issues: FCSbookTripleInstList will have more instances
}

//Role list is not direct only
//Concept sub is direct only for triple
//ontology has inverse
//concept of inst is not direct only
