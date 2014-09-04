/*
 * Klasasafn sem nota má til að smíða milliþulu fyrir Schmorpho
 */

#ifndef __CODEGEN_H__
#define __CODEGEN_H__

#include <iostream>
#include <vector>
#include <map>
#include <string>

class EDef;
class EDefFun;
class EDefVar;
class EExpr;

// Hjálparklasi fyrir stofn falls í Bison
struct BodyData {
	~BodyData() {
		delete funvec;
		delete varvec;
	}
	std::vector<EDefFun *> *funvec;
	std::vector<EDefVar *> *varvec;
	EExpr *expr;
};

// Allt er Node
struct Node {
	// smíðar nýjan hnút með engar upplýsingar
	Node() :
		container(0), index(0), origin(0) {
	}
	virtual ~Node();

	// skrifa þulu fyrir hnútinn þannig að gildi hans verði á Acc
	virtual void generateAcc(std::ostream& out) const = 0;
	// uppfæra tréð í hnútnum þannig að cnt verði umlykjandi skilgreining
	virtual void setContainer(EDefFun *cnt);
	// uppfæra tréð í hnútnum þannig að orig verði hnit uppruna þess í container
	virtual void setOrigin(int orig);

	std::vector<Node *> children; // vigur barna hnútarins
	EDefFun *container; // bendir upp í umlykjandi skilgreiningu
	int index;  // fjöldi systkina á undan hnútnum
	int origin; // hnit uppruna hnútarins í container
};

// Yfirklasi skilgreininga
struct EDef: public Node {
	std::string name; // nafnið sem container tengir við hlutinn
};

// Skilgreining á falli
class EDefFun: public EDef {
public:
	// smíðar nýjan hnút sem stendur fyrir skilgreiningu á falli með nafnið í name,
	// viðfanganöfnin í args og stofninn í body
	EDefFun(std::string *name, std::vector<std::string *> *args, BodyData *body);

	// Skrifa þulu fyrir fallið sem ysta fall
	void generateFun(std::ostream& out) const;
	// Skrifa þulu fyrir vistun á lokun fallsins á hæð sinni
	void generateAcc(std::ostream& out) const;

	void setContainer(EDefFun *cnt);
	void setOrigin(int orig);

	// Leitar að breytunafni name frá sjónarhorni upprunans orig
	// á þessari og ytri földunarhæðum þannig. Eftir kallið verður dist fjöldi
	// „veggja“ sem fara þurfti gegn um við leitina, fallið skilar bendi
	// á nálægustu sýnilegu skilgreiningu nafnsins. Ef sá bendir er 0 þá
	// þýðir það að nafnið var hvergi sýnilegt.
	EDef *locateName(const std::string& name, int orig, int& dist);
protected:
	int argc; // fjöldi viðfanga fallsins
	int func; // fjöldi innri falla á földunarhæðinni sem þetta fall skilgreinir
	std::map<std::string, EDef *> defs; // nöfn á þessari földunarhæð
};

// Skilgreining á staðværri breytu
class EDefVar: public EDef {
public:
	// Smíðar rót fyrir nýja breytuskilgreiningu með nafnið name.
	// Ef ekkert undirtré er gefið, þá þýðir það bara að nafnið er
	// frátekið, sem getur aðeins gerst ef það er viðfang í skilgr. falls.
	EDefVar(std::string *name, EExpr *val = 0) :
		value(val) {
		this->name = *name;
		if (value) {
			children.push_back((Node *) value);
		}
	}
	void generateAcc(std::ostream& out) const;
protected:
	EExpr *value; // tréð sem þýðist í gildi breytunnar

};

// Yfirklasi segða
class EExpr: public Node {
public:
	// Skrifar þulu á out fyrir segðina sem vistar gildi hennar í sætið
	// pos í vakningarfærslu með númerið ar
	virtual void generateArg(std::ostream& out, int ar, int pos) const;
	// Skrifar þulu á out, sem gefur vakningarfærslunni
	// sem á að fá gildið úr fallinu, sem inniheldur segðina fyrir hlutinn,
	// stjórn.
	virtual void generateReturn(std::ostream& out) const;
};

// if setning
class EIf: public EExpr {
public:
	// Smíðar tré fyrir if-segð af gerðinni (if cond ifexpr elseexpr)
	EIf(EExpr *cond, EExpr *ifexpr, EExpr *elseexpr) :
		condition(cond), ifexpr(ifexpr), elseexpr(elseexpr) {
		children.push_back(cond);
		children.push_back(ifexpr);
		children.push_back(elseexpr);
	}
	void generateAcc(std::ostream& out) const;
	void generateReturn(std::ostream& out) const;
protected:
	EExpr *condition,  // tréð fyrir skilyrðið
			*ifexpr,   // tréð fyrir ef-satt segðina
			*elseexpr; // tréð fyrir else-segðina
};

// yfirklasi kalla á föll
class EFunCall: public EExpr {
public:
	virtual void generateReturn(std::ostream& out) const;
	virtual void generateAcc(std::ostream& out) const;
	virtual void generateCall(std::string& type, std::ostream& out) const=0;
};

// kall á fall gefið með breytunafni
class ENameCall: public EFunCall {
public:
	// Smíðar tré fyrir kall á fall með nafn name og gildisviðföng args
	ENameCall(std::string *name, std::vector<EExpr *> *args) :
		name(*name) {
		for (int i = 0; i < args->size(); i++) {
			children.push_back((*args)[i]);
			children.back()->index = i;
		}
	}
	// Skrifar þulu á out fyrir kall á fallið sem hluturinn stendur fyrir
	// þar sem type má vera "Become" fyrir halaendurkvæmt kall eða
	// "Call" fyrir venjulegt kall sem setur gildi á hlaðann.
	void generateCall(std::string& type, std::ostream& out) const;
protected:
	std::string name;
};

// kall fall gefið með segð (alltaf lokun)
class EExprCall: public EFunCall {
public:
	// Smíðar tré fyrir kall á lokun sem reiknast út úr closure og tekur
	// gildisviðföngin args
	EExprCall(EExpr *closure, std::vector<EExpr *> *args) :
		closure(closure) {
		for (int i = 0; i < args->size(); i++) {
			children.push_back((*args)[i]);
			children.back()->index = i;
		}
		closure->index = children.size();
		children.push_back(closure);
	}
	// eins og í EFunCall
	void generateCall(std::string& type, std::ostream& out) const;
protected:
	EExpr *closure;
};

// frumstæð gildi í morpho þulunni (engin börn)
class EVal: public EExpr {
public:
	// Smíðar hlut sem stendur fyrir gildið í val
	EVal(std::string *value) :
		value(*value) {
	}
	void generateAcc(std::ostream& out) const;
	void generateArg(std::ostream& out, int ar, int pos) const;
protected:
	std::string value; // strengur fyrir gildið
};

// vísun í breytunafn eftir skilgreiningu (engin börn)
class EVar: public EExpr {
public:
	// Smíðar hlut sem stendur fyrir vísun í breytu með nafnið í name
	EVar(std::string *name) :
		name(*name) {
	}
	void generateAcc(std::ostream& out) const;
	void generateArg(std::ostream& out, int ar, int pos) const;
protected:
	std::string name; // nafnið sem vísað er í
};

#endif
