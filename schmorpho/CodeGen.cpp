#include "CodeGen.h"

// teljarar fyrir tréð
static int nextLabel = 0, nextArPos = -1;

int newLabel()    { return nextLabel++; } // nýtt númer límmiða
int pushAr()      { return nextArPos--; } // fá nýja vakningarfærslu
int popAr()       { return nextArPos++; } // skila vakningarfærslu

// Destructors

Node::~Node() {
	while(!children.empty()) {
		delete children.back();
		children.pop_back();
	}
}

// Constructors

EDefFun::EDefFun(std::string *name, std::vector<std::string *> *args,
		BodyData *body) :
	argc(args->size()), func((body->funvec)?body->funvec->size():0) {
	this->name = std::string(*name);

	// Fyrir hvert nýtt barn sem við bætum við í children
	// uppfærum við um leið tilviksbreytur foreldrisins og þess
	// undirtrés, sem barnið stendur fyrir, í samræmi við nýtt
	// ástand trésins.

	// viðföng eru tómar breytuskilgreiningar
	for (int i = 0; i < args->size(); i++) {
		EDefVar *arg = new EDefVar((*args)[i]);
		arg->setContainer(this);
		arg->index = i;
		arg->setOrigin(i);
		children.push_back(arg);
		defs[arg->name] = arg;
	}
	// staðvær föll
	std::vector<EDefFun *> *funvec = body->funvec;
	for (int i = 0; funvec && i < funvec->size(); i++) {
		EDefFun *d = (*funvec)[i];
		d->setContainer(this);
		d->index = children.size();
		d->setOrigin(d->index);
		children.push_back(d);
		defs[d->name] = d;
	}
	// staðværar breytur
	std::vector<EDefVar *> *varvec = body->varvec;
	for (int i = 0; varvec && i < varvec->size(); i++) {
		EDefVar *v = (*varvec)[i];
		v->setContainer(this);
		v->index = children.size();
		v->setOrigin(v->index);
		children.push_back(v);
		defs[v->name] = v;
	}
	// eigum bara eftir að afgreiða skilasegðina
	body->expr->setContainer(this);
	body->expr->index = children.size();
	body->expr->setOrigin(body->expr->index);
	children.push_back(body->expr);
}

// Hjálparföll fyrir bókhald og þulusmíði

void Node::setContainer(EDefFun *cnt) {
	// öll börn eru af gerð EExpr og hafa þar með
	// sameiginlegan container
	container = cnt;
	for (int i = 0; i < children.size(); i++) {
		children[i]->setContainer(cnt);
	}
}

void Node::setOrigin(int orig) {
	origin = orig;
	for(int i = 0; i < children.size(); i++) {
		children[i]->setOrigin(orig);
	}
}

void EDefFun::setContainer(EDefFun *cnt) {
	// container fyrir EDefFun er alltaf EDefFun og
	// EDefFun er alltaf container barna sinna
	container = cnt;
}
void EDefFun::setOrigin(int orig) {
	// uppruni nafns EDefFun er alltaf hluturinn sjálfur
	origin = orig;
}

EDef *(EDefFun::locateName)(const std::string& sName, int orig, int& dist) {
	EDefFun *level = this;
	dist = 0; int origin = orig;
	while(level) {
		// búið er leita dist á dist hæðum í skilgreiningatrénu
		// i er númer uppruna kallsins á level
		EDef *val = level->defs[sName];
		if(val && ((val->index <= level->func+level->argc) || (val->index < origin) ))
			return val;
		origin = level->index;
		level = level->container;
		dist++;
	}
	return 0;
}

// Þulusmíði

void EDefFun::generateFun(std::ostream& out) const {
	out << "#\"" << name << "[f"<< argc << "]\"=";
	out << "\n[";
	for(int i = 0; i < children.size()-1; i++)
		children[i]->generateAcc(out);
	((EExpr *) children.back())->generateReturn(out);
	out << "\n];" << std::endl;
}

void EDefFun::generateAcc(std::ostream& out) const {
	// (MakeClosure target lev nparams); hér er alltaf lev = 0
	int target = newLabel(), jump = newLabel();
	out << "\n(MakeClosure _" << target << " 0 " << this->argc << ")";
	out << "\n(Go _" << jump << ")";
	out << "\n_" << target << ":";
	// Þulan fyrir meginmálið
	for(int i = 0; i < children.size()-1; i++) {
		// búið að skrifa þuluna fyrir i fyrstu börnin
		children[i]->generateAcc(out);
	}
	((EExpr *) children.back())->generateReturn(out);
	out << "\n_" << jump << ":";
	out << "\n(Store 0 " << this->index << ")";
}

void EDefVar::generateAcc(std::ostream& out) const {
	// skrifum aðeins þulu breytuskilgreiningar sem eru ekki viðföng falla
	// value != 0 eff það er bendir á barn EDefVar (sem getur mest verið eitt)
	if(value) {
		value->generateAcc(out);
		out << "\n(Store 0 " << this->index << ")";
	}
}

void EExpr::generateArg(std::ostream& out, int ar, int pos) const {
	generateAcc(out);
	out << "\n(StoreArgAcc " << ar << " " << pos << ")";
}
void EExpr::generateReturn(std::ostream& out) const {
	generateAcc(out);
	out << "\n(Return 0)";
}

void EIf::generateAcc(std::ostream& out) const {
	int gofalse = newLabel();
	condition->generateAcc(out);
	out << "\n(GoFalse _" << gofalse << ")";
	ifexpr->generateAcc(out);
	out << "\n_" << gofalse << ":";
	elseexpr->generateAcc(out);
}
void EIf::generateReturn(std::ostream& out) const {
	// EIf áframsendir generateReturn á greinar sínar (en ekki skilyrðið)
	int gofalse = newLabel();
	condition->generateAcc(out);
	out << "\n(GoFalse _" << gofalse << ")";
	ifexpr->generateReturn(out);
	out << "\n_" << gofalse << ":";
	elseexpr->generateReturn(out);
}


void EFunCall::generateAcc(std::ostream& out) const {
	std::string type = "Call";
	generateCall(type, out);
}

void EFunCall::generateReturn(std::ostream& out) const {
	std::string type = "Become";
	generateCall(type, out);
}

void ENameCall::generateCall(std::string& type, std::ostream& out) const {
	// undirbúum vakningarfærsluna
	int ar = pushAr(); // ar er númer vakningarfærslunnar
	for(int i = 0; i < children.size(); i++) {
		((EExpr *) children[i])->generateArg(out, ar, i);
	}
	// kall á fallið sjálft
	int lev;
	EDef *callDef = container->locateName(name,origin,lev);
	if(callDef && (callDef->container)) {
		// callDef er staðvær breyta
		out << "\n(Fetch " << lev << " " << callDef->index << ")";
		out << "\n("<< type <<"Closure " << children.size() << " " << ar << ")";
	}
	else {
		// callDef er skilgreining á ytra falli
		out << "\n("<< type <<" " << "#\"" << name <<"[f"<< children.size() << "]\" " << ar << ")";
	}
	popAr();
}

void EExprCall::generateCall(std::string& type, std::ostream& out) const {
	// undirbúum vakningarfærsluna
	int ar = pushAr(); // ar er númer vakningarfærslunnar
	for(int i = 0; i < children.size()-1; i++) {
		((EExpr *) children[i])->generateArg(out, ar, i);
	}
	// þýðum segðina sem stendur fyrir lokunina
	closure->generateAcc(out);
	// lokunin fyrir fallið er á hlaðanum
	out << "\n("<< type <<"Closure " << (children.size()-1) << " " << ar << ")";
	popAr();
}

void EVar::generateAcc(std::ostream& out) const {
	int lev = 0;
	EDef *varDef = container->locateName(name,origin,lev);
	if(varDef && (varDef->container)) {
		out << "\n(Fetch " << lev << " " << varDef->index << ")";
	}
	else {
		// villa; nafnið verður að vera skilgreint e-s staðar
		std::cerr << "\nVilla: Óþekkt nafn \"" << name << "\"" ;
	}
}

void EVar::generateArg(std::ostream& out, int ar, int pos) const {
	int lev = 0;
	EDef *varDef = container->locateName(name,origin,lev);
	if(varDef && (varDef->container)) {
		out << "\n(StoreArgVar " << ar << " " << pos << " " << lev << " " << varDef->index << ")";
	}
	else {
		// villa; nafnið verður að vera skilgreint e-s staðar
		std::cerr << "\nVilla: Óþekkt nafn \"" << name << "\"";
	}
}

void EVal::generateAcc(std::ostream& out) const {
	out << "\n(MakeVal " << value << ")";
}

void EVal::generateArg(std::ostream& out, int ar, int pos) const {
	out << "\n(StoreArgVal " << ar << " " << pos << " " << value << ")";
}
