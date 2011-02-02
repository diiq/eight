import re

class Interior():
    def __init__(self, thing):
	self.value = thing
	self.info = {}

class EObject():
    def __init__(self, inner, atype):
	self.bindings = []
	self.inner = Interior(inner)
	self.inner.info['type'] = atype


    def type(self):
	return self.inner.info['type']

    def copy(self):
	ret = EObject(self.inner.value, self.type())
	ret.inner.info = self.inner.info
	ret.bindings = self.bindings
	return ret

class Operation():
    def __init__(self, next, instruction, flag):
	self.next = next
	self.instruction = instruction
	self.flag = flag

class Frame():
    def __init__(self, below, name):
	self.below = below
	self.next = None
	self.scope = {}
	self.rib = {}
	self.signal_handler = None
	self.trace = name
    

class Machine():
    def __init__(self):
	self.base_frame =  Frame(None, symbol("base-frame"))
	self.current_frame = self.base_frame
	self.accum = None
	self.paused = True
    

#--------------------------------- coo.

nil = EObject(None, "nil")

def nilp(x):
    return (x.type() == "nil")
    

class Cons_cell():
    def __init__(self, car, cdr):
	self.car = car
	self.cdr = cdr
    

def cheap_cons(car, cdr):
    return EObject(Cons_cell(car, cdr), "cons")
    

def symbol(name):
    return EObject(name, "symbol")
    

def estring(str):
    return EObject(str, "string")
    

#------------- Binding algebra ----------------------#

def combine_bindings(a, b, newa, newb):
    c = {}
    for sym in a:
        if sym in b:
            if a[sym] == b[sym]: # do I mean equal?
                c[sym] = a[sym]
            else :
                newa[sym] = a[sym]
                newb[sym] = b[sym]	
        else :
            c[sym] = a[sym]
	    
    for sym in b:
        if not sym in a:
            c[sym] = b[sym]
	    	
    return c
    

def cons(car, cdr):
    ncar = car.copy()
    ncdr = cdr.copy()

    if (nilp(car)):
        ret = cheap_cons(nil, ncdr)
        ret.bindings = ncdr.bindings
        ncdr.bindings = {}
        return ret
	

    if (nilp(cdr)):
        ret = cheap_cons(ncar, nil)
        ret.bindings = ncar.bindings
        ncar.bindings =  {}
        return ret
	

    ncar.bindings =  {}
    ncdr.bindings =  {}
    ret = cheap_cons(ncar, ncdr)
    ret.bindings = combine_bindings(car.bindings, cdr.bindings,
                                    ncar.bindings, ncdr.bindings)
    return ret
    

def cheap_car(x):
    if (nilp(x)):
        return nil   
    return x.inner.value.car
    

def cheap_cdr(x):
    if (nilp(x)):
        return nil
    return x.inner.value.cdr
    

def union_bindings(a, b):
    ret =  {}
    for sym in a:
        ret[sym] = a[sym]
	
    for sym in b:
        if not leakedp(b[sym]):
            ret[sym] = b[sym]
	    	
    return ret
    


def car(x):
    if nilp(x):
        return nil
	
    if not x.inner.value.__class__ == Cons_cell:
        print "Car of a thing not consed!",  stringify(x)

    ret = cheap_car(x).copy()
    ret.bindings = union_bindings(x.bindings, ret.bindings)
    return ret
    

def cdr(x):
    if nilp(x):
        return nil

    if not x.inner.value.__class__ == Cons_cell:
        print "Cdr of a thing not consed!",  stringify(x)
        
    ret = cheap_cdr(x).copy()
    ret.bindings = union_bindings(x.bindings, ret.bindings)
    return ret
    

#---------------- OK. -------------------#

def cheap_elist(*args):
    ret = nil
    for i in range(len(args)-1, -1, -1):
        ret = cons_pair(args[i], ret)	
    return ret

def elist(*args):
    ret = nil
    for i in range(len(args)-1, -1, -1):
        ret = cons(args[i], ret)	
    return ret
    
def append(x, y):
    # Currently possible to blow the stack.
    if nilp(x):
        return y
    return cons(car(x), append(cdr(x), y))
    

#--------------- symbol binding shit --------------#


def leaked():
    return elist(symbol("leak"))
    

def leakedp(x):
    if (x and
        x.type() == "cons" and
        cheap_car(x).type() == "symbol" and
        cheap_car(x).inner.value == "leak"):
        return True
    return False
    

def look_up(sym, m):
    if (sym.inner.value in sym.bindings): # check in the local closing
        ret = sym.bindings[sym.inner.value]
    else:
        ret = leaked()
	
    current_frame = m.current_frame

    while leakedp(ret):  # in the current frame (and below)
        if sym.inner.value in current_frame.scope:
            ret = current_frame.scope[sym.inner.value]
            current_frame = current_frame.next
        else:
            ret = nil
	
    if ret == nil: # and in the base frame.
        if sym.inner.value in m.base_frame.scope:
            ret = m.base_frame.scope[sym.inner.value]
	
    return ret
    


def set(sym, val, m):
    oldval = look_up(sym, m)
    if nilp(oldval):
        m.base_frame.scope[sym.inner.value] = elist(val)
    else:
        oldval.inner.value.car = val
	
    




#-------- machine shit -----------------#

def asterixp(x):
    if (x.type() == "cons" and
	    cheap_car(x).type() == "symbol" and
	    cheap_car(x).inner.value == "*"):
	    return True	
    return False
    

def atpendp(x):
    if (x.type() == "cons" and
        cheap_car(x).type() == "symbol" and
        cheap_car(x).inner.value == "@"):
        return True	
    return False
    

def elipsisp(x):
    return (x.type() == "symbol" and
            x.inner.value == "...")
    

def argument_chain(lambda_elist, arg_elist, chain):
    if lambda_elist.type() == "nil":
        return chain

    arg = car(arg_elist)
    if (asterixp(arg)):
        last =  Operation(chain,
                          elist(lambda_elist, cdr(arg_elist)),
                          "asterpend_continue")
        ret =  Operation(last, car(cdr(arg)), "evaluate")

    elif (atpendp(arg)):
        last =  Operation(chain,
                          elist(lambda_elist, cdr(arg_elist)),
                          "atpend_continue")
        ret =  Operation(last, car(cdr(arg)), "evaluate")

    else:
        alambda = car(lambda_elist)
        flag = "argument"

        if (elipsisp(alambda)):
            if (nilp(arg_elist)):
                return chain
		
            alambda = car(cdr(lambda_elist))
            lambda_elist = cons(nil, lambda_elist)
            flag = "e_argument"
        
        if (alambda.type() == "cons"):
            name = car(cdr(alambda))
            if (arg.type() == "cons" and
                car(arg).type() == "symbol" and
                car(arg).inner.value == ","):
                
                func = car(cdr(arg))
            else:
                func  = elist(car(alambda), arg)
		
        else:
            name = alambda
            func = arg
	    
        last = Operation(chain,
                         elist(cdr(lambda_elist), cdr(arg_elist)),
                         "continue")
        ret = Operation(Operation(last, name, flag),
                        func,
                        "evaluate")	
    return ret
    


def clear_elist(x):
    if nilp(x):
        return nil
    return cons(elist(symbol("clear"), car(x)), clear_elist(cdr(x)))
    

def machine_step(m):
    # this is the heart of the eight machine
    # and if an imperative def this long doesn't make you
    # at least a little squeamish, you're not human.
#    print "Accum:", stringify(m.accum)
#    print stringify(m.current_frame)
#    print "\n\n\n"

    if (not m.current_frame.next):   # this block returns
        # The current frame contains no further instructions.
        if (m.current_frame.below != None):
            # Move on to the next frame.
            m.current_frame = m.current_frame.below
            return
        else :
            # The frame below is the base frame, so we must be done.
            m.paused = True
            return    

    instruction = m.current_frame.next
    m.current_frame.next = instruction.next

    if (instruction.flag == "evaluate"):
        if (instruction.instruction.type() == "builtin"):
            instruction.instruction.inner.value(m)

        elif (instruction.instruction.type() == "symbol") :
		#alert("here")
 		#alert(stringify(car(look_up(instruction.instruction, m))))
            ret  = look_up(instruction.instruction, m)
            if (nilp(ret)):
		    #signal
                print ("thass no' a symbol, lassie: ",
                      stringify(instruction.instruction))
                print stringify(m.current_frame);
                return 
            m.accum = car(ret)

        elif instruction.instruction.bindings:
            m.current_frame.scope = union_bindings(
                m.current_frame.scope,
                instruction.instruction.bindings)
            nex = instruction.instruction.copy()
            nex.bindings =  {}
            m.current_frame.next =  Operation(m.current_frame.next,
                                              nex,
                                              "evaluate")
            
        elif (instruction.instruction.type() == "cons") :
            if (car(instruction.instruction).type() == "symbol" and
                car(instruction.instruction).inner.value == "clear"):
                m.accum = car(cdr(instruction.instruction))
                
            else:
                name = instruction.instruction
                m.current_frame =  Frame(m.current_frame, name)
                m.current_frame.scope = m.current_frame.below.scope
                m.current_frame.next =  Operation(
                    Operation(
                        m.current_frame.next,
                        cdr(instruction.instruction),
                        "apply"),
                    car(instruction.instruction),
                    "evaluate")
		

        else:
		# It's some CRAZY type, amirite? Let's not evaluate it,
		# and hate it because it's different.
		m.accum = instruction.instruction
	    
    else:
        # So we must be dealing with a fancy flag.
        if (instruction.flag == "apply"):
            m.current_frame.next =  Operation(m.current_frame.next,
                                              cdr(m.accum),
                                              "do")
            m.current_frame.next = argument_chain(car(m.accum),
                                                  instruction.instruction,
                                                  m.current_frame.next)
            
        elif (instruction.flag == "continue"):
            m.current_frame.next = argument_chain(
                car(instruction.instruction),
                car(cdr(instruction.instruction)),
                m.current_frame.next)
            
        elif (instruction.flag == "asterpend_continue"):
            m.current_frame.next = argument_chain(
                car(instruction.instruction),
                append(m.accum, car(cdr(instruction.instruction))),
                m.current_frame.next)

        elif (instruction.flag == "atpend_continue"):
            m.current_frame.next = argument_chain(
                car(instruction.instruction),
                append(clear_elist(m.accum),
                       car(cdr(instruction.instruction))),
                m.current_frame.next)
            
        elif (instruction.flag == "argument"):
            m.current_frame.rib[instruction.instruction.inner.value] = elist(m.accum)
            
        elif (instruction.flag == "e_argument"):
            sym = instruction.instruction.inner.value
            if (sym in m.current_frame.rib):
                so_far = m.current_frame.rib[sym]
                so_far = append(car(so_far), elist(m.accum))
                m.current_frame.rib[sym] = elist(so_far)
            else :
                m.current_frame.rib[sym] = elist(elist(m.accum))
		
                
        elif (instruction.flag == "do") :
            m.current_frame.scope = m.current_frame.rib
            temp = m.current_frame.next
            chain = m.current_frame
            while not nilp(instruction.instruction):
                chain.next =  Operation(m.current_frame.next,
                                        car(instruction.instruction),
                                        "evaluate")
                chain = chain.next
                instruction.instruction = cdr(instruction.instruction)
		
		chain.next = temp
                
                
                
                
                
def copy_frame(x) :
    if (x.below):
        below = copy_frame(x.below)
    else :
        return x
    
    ret =  Frame(below, x.trace)
    ret.next = x.next
    ret.scope = x.scope.copy()
    ret.rib = x.rib.copy()
    ret.signal_handler = x.signal_handler
    return ret


def build_continuation(m):
    q =  Machine()
    c_frame = copy_frame(m.current_frame)
    def f(m):
        val = get_arg("val", m)
        m.current_frame = c_frame
        m.accum = val    
    return elist(elist(symbol("val")),  EObject(f, "builtin"))


def signal(message, m):
    sig = elist(build_continuation(m), message)
    toss_signal(sig, m)
    

def toss_signal(signal, m):
    m.current_frame = m.current_frame.below
    while not m.current_frame.signal_handler and m.current_frame.below:
        m.current_frame = m.current_frame.below
	
    if not m.current_frame.signal_handler and not m.current_frame.below:
        print "No BASE SIGNAL HANDLER grr."
    else :
        m.current_frame.next =  Operation(m.current_frame.next, 
                                          elist(m.current_frame.signal_handler, 
                                                elist(symbol("clear"), signal)), 
                                          "evaluate")

#--------------- building it in -----------------#

def add_fn(m, name, alambda, fn):
    fun = elist(parse(preparse(alambda)),  EObject(fn, "builtin"))
    fun.inner.info["def-name"] = symbol(name)
    m.base_frame.scope[name] = elist(fun)
    

def get_arg(x, m):
    return car(look_up(symbol(x), m))


def add_em_all(m):
    def bcons(m):
        cdr =  get_arg("cdr", m)
        if (not (cdr.inner.value.__class__ == Cons_cell) and
            not nilp(cdr)):
            alert("Warning that you should make real elists!")
        m.accum = cons(get_arg("car", m), cdr)

    add_fn(m, "cons", "(car cdr)", bcons)

    def bcar(m):
        x =  get_arg("x", m)
        m.accum = car(x)

    add_fn(m, "car", "(x)", bcar)

    def bcdr(m):
        x =  get_arg("x", m)
        m.accum = cdr(x)

    add_fn(m, "cdr", "(x)", bcdr)

    def binfo(m):
        x =  get_arg("x", m)
        m.accum =  EObject(x.inner.info, "table")

    add_fn(m, "info", "(x)", binfo)
    
    def breakb(m):
        m.paused = True
    add_fn(m, "break", "()", breakb)

    def bquote(m):
        x =  get_arg("x", m)
        temp = m.current_frame
        m.current_frame = temp.below
        m.accum = enclose(x, m)
        m.current_frame = temp

    add_fn(m, "'", "((clear x))", bquote)

    def bset(m):
        sym =  get_arg("symbol", m)
        val =  get_arg("value", m)
        set(sym, val, m)
        m.accum = val

    add_fn(m, "set!", "('symbol value)", bset)

    def bprint(m):
        val =  get_arg("thing", m)
        print stringify(val),
        m.accum = val
        
    add_fn(m, "print", "(thing)", bprint)
    
    def bleak(m):
        sym =  get_arg("symbol", m)
        val =  get_arg("expression", m)
        val.bindings[sym.inner.value] = elist(symbol("leak"))
        m.accum = val

    add_fn(m, "leak", "(symbol expression)", bleak)

    def atom_pb(m): # what, lead?
        x =  get_arg("x", m)
        if (x.type() == "cons"):
            m.accum = nil
        else :
            m.accum = symbol("t")
 
    add_fn(m, "atom-p", "(x)", atom_pb)

    def oifb(m):
        test =  get_arg("test", m)
        if not nilp(test):
            m.current_frame.next =  Operation(
                m.current_frame.next,
                get_arg("then", m),
                "evaluate")
        else :
            m.current_frame.next =  Operation(
                m.current_frame.next,
                get_arg("else", m),
                "evaluate")
            
    add_fn(m, "oif", "(test 'then 'else)", oifb)
    
    def isb(m): # brother to canby
        a =  get_arg("a", m)
        b =  get_arg("b", m)
        if (a == b or
            (a.type() == b.type() and
             a.inner.value == b.inner.value)) :
            m.accum = symbol("t")
        else :
            m.accum = nil
            
    add_fn(m, "is", "(a b)", isb)
            
    def plusb(m):
        a =  get_arg("a", m)
        b =  get_arg("b", m)
        m.accum =  EObject(a.inner.value +
                           b.inner.value,
                           "number")
        
    add_fn(m, "plus", "(a b)", plusb)
        
    def callccb(m): # That's 95533
        a =  get_arg("def", m)
        m.current_frame.next =  Operation(
            m.current_frame.next,
            elist(a, elist(symbol("clear"),
                           build_continuation(m))),
            "evaluate")
        
    add_fn(m, "call/cc", "('def)", callccb)

    def signalb(m):
        a =  get_arg("message", m)
        signal(a, m)
        
    add_fn(m, "signal", "(message)", signalb)
        
    def handlesignalsb(m):
        a =  get_arg("def", m)
        m.current_frame.signal_handler = a
        m.current_frame.next =  Operation(
            m.current_frame.next,
            get_arg("rest", m),
            "do")
        
    add_fn(m, "handle-signals", "('def ... 'rest)", handlesignalsb)

    def basehandlerb(m):
        a =  get_arg("def", m)
        m.base_frame.signal_handler = a
        m.accum = a
        
    add_fn(m, "base-signal-handler", "('def)", basehandlerb)

    def unhandleb(m):
        a =  get_arg("signal", m)
        toss_signal(a, m)
        toss_signal(a, m)
        
    add_fn(m, "unhandle-signal", "(signal)", unhandleb)


def enclose(code, m):
    code.bindings =  {}
    find_free_variables(code, code.bindings, m)
    return code


def free_var_p(sym, bindings):
    return (sym.type() == "symbol" and not (sym.inner.value in bindings))

def find_free_variables(code, bindings, m):
    if (free_var_p(code, bindings)):
        a = look_up(code, m)
        if not nilp(a):
            bindings[code.inner.value] = a
	    
    elif code.type() == "cons" :
        find_free_variables(car(code), bindings, m)
        find_free_variables(cdr(code), bindings, m)
	
    

def init_machine():
    m =  Machine()
    m.base_frame.scope["q"] = elist(symbol("sq"))
    m.base_frame.scope["r"] = elist(symbol("sr"))
    m.base_frame.scope["s"] = elist(symbol("ss"))
    add_em_all(m)
    return m
    


def execute(str, m):
    op =  Operation(None, ultraparse(str), "evaluate")
    m.current_frame =  Frame(m.base_frame, symbol("initial"))
    m.current_frame.next = op
    m.paused = False
    
    while(m.paused == False):
        machine_step(m)

    return m.accum
    

def load(str, m):
    lamda = ultraparse(str)
    op =  Operation(None, lamda, "evaluate")
    m.current_frame =  Frame(m.base_frame, symbol("initial"))
    m.current_frame.next = op
    m.paused = False
    

def step(m):
    m.paused = False
    machine_step(m)
    

def continu(m):
    m.paused = False
    while(m.paused == False):
        machine_step(m)
	
    
        
# ------------------------------ Parsing ----------------------------#
# This is a bit hacky, yes? TODO: come back and write a nice parser.


def ultraparse(astr):
    return parse(preparse("((clear (() "+astr+")))"))


def preparse (astr):
    astr = astr.split("\"")
    aastr =  []
    for a in range(len(astr)):
        if (a % 2 == 0):
            aastr.extend(tokenize(astr[a]))
        else :
            aastr.append("\"" + astr[a] + "\"")
	   
    aastr.reverse()         
    return aastr
    

def tokenize(astr):
    astr = re.sub("\#.*\n", " ", astr)

    astr = re.sub("\(", "( ", astr)    
    astr = re.sub("[^'@*,]\(", " (", astr)
    astr = re.sub("\)", " ) ", astr)
    
    astr = astr.split()
    out =  []
    for token in astr:
        if token:
            out.append( token)
	            
    return out
    

def parse(tokens):
    token = tokens.pop()

    if (token == "("):
        return parse_elist(tokens)
    
    elif (token[0] == "'" and len(token) > 1):
        tokens.append(token[1:])
        return elist(symbol("'"), parse(tokens))
    
    elif (token[0] == "@" and len(token) > 1):
        tokens.append(token[1:])
        return elist(symbol("@"), parse(tokens))
    
    elif (token[0] == "*" and len(token) > 1):
        tokens.append(token[1:])
        return elist(symbol("*"), parse(tokens))
    
    elif (token[0] == "," and len(token) > 1):
        tokens.append(token[1:])
        return elist(symbol(","), parse(tokens))
    
    elif (token[0] == "\""):
        return estring(token[1:-1])
    
    elif (re.match("^\-?\d+\.?\d*?$", token)):
        return  EObject(float(token), "number")
    
    else :
        return symbol(token)    
    

def parse_elist(tokens):
    lis = []
    while tokens[-1] != ")":
        lis.append(parse(tokens))
    tokens.pop()
    return elist(*lis)
    

#------------------------- Unparsing -------------------------#


def stringify(x):
    if not x:
        return "NONE"
    elif (x.__class__ == EObject):
        return stringify_eobject(x)
    elif (x.__class__ == Operation):
        return stringify_operation(x)
    elif (x.__class__ == Frame):
        return stringify_frame(x)
    else:
        alert("I don't know how to astringiy that.")
        return "STRINGIFY ERR"
	

def stringify_frame(x):
    if not x:
        return ""
    else:
        return (stringify(x.trace) +
                "\n scoped as " + stringify_scope(x.scope) +
                "\n ribbed as " + stringify_scope(x.rib) +
                "\n performing:\n" +
                stringify_operation(x.next) +
                "\n\n" + stringify_frame(x.below))
    
    
def stringify_scope(x):
    ret = ""
    for a in x:
        ret += "\n" + a + " -> " + stringify(x[a])	
    return ret
    

def stringify_operation(x):
    if x == None:
        return ""
    else :
        return ("|-> " + stringify(x.instruction) + ", flagged " + x.flag +
                "\n" + stringify(x.next))
	
    
def stringify_eobject(x):
    type = x.type()
    if type == "cons":
        ret = "(" + stringify_elist(x) + ")"
    elif (type == "nil"):
        ret = "()"
    elif (type == "astring"):
        ret = "\"" + x.inner.value + "\""
    elif (type == "builtin"):
        ret = "BUILTIN"
    else :
        ret = x.inner.value.__str__()
    return ret
    

def stringify_elist(x):
    if (x.inner.value.cdr.type() == "nil"):
        return stringify(x.inner.value.car)
    else :
        return (stringify(x.inner.value.car) + " " +
                stringify_elist(x.inner.value.cdr))
