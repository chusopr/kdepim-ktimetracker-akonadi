// XXX Automatically generated. DO NOT EDIT! XXX //

RDispositionType();
RDispositionType(const RDispositionType &);
RDispositionType(const QCString &);
RDispositionType & operator = (const RDispositionType &);
RDispositionType & operator = (const QCString &);
bool operator == (RDispositionType &);
bool operator != (RDispositionType & x) { return !(*this == x); }
bool operator == (const QCString & s) { RDispositionType a(s); return (*this == a); } 
bool operator != (const QCString &s) {return !(*this == s);}

virtual ~RDispositionType();
void _parse();
void _assemble();
void parse() 			{ if (!parsed_) _parse(); parsed_ = true; assembled_ = false; }

void assemble() 			{ parse() ; if (!assembled_) _assemble(); assembled_ = true;}

void createDefault();

const char * className() const { return "RDispositionType"; }

// End of automatically generated code           //
