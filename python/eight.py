# Eight. 2/2/11 S.B.

class Interior():
    # Interiors exist to ensure one step of removal between values
    # (which reside in Interiors) and bindings. This allows the same
    # value to have multiple bindings.
    def __init__(self, thing):
	self.value = thing
	self.info = {}

class EObject():
    # Eight manipulates EObjects; all EObjects include a list of
    # bindings, symbol to value. Usually, that list is empty. The type
    # of an EObject is stored, with the value, in its inner.
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
    # Operations represent individual instructions to the Eight
    # machine. They contain a link to the subsequent instruction, an
    # EObject to be manipulated, and a flag, which is a string telling
    # the machine what action to perform. Easy-peasy, right?
    def __init__(self, next, instruction, flag):
	self.next = next
	self.instruction = instruction
	self.flag = flag

class Frame():
    # Frames are a list of Operations in a specific scope. They stack
    # in a linked list. Rib is just a place to put the scope that is
    # being built, before it is being used. 
    def __init__(self, below, name):
	self.below = below
	self.next = None
	self.scope = {}
	self.rib = {}
	self.signal_handler = None
	self.trace = name

class Machine():
    # The whole state of the machine is represented with the
    # frame-stack, an accumulator value that represents the
    # most-recently-calculated value, and a convenience variable to
    # pause the machine for debugging, or when a calculation is
    # complete.
    def __init__(self):
	self.base_frame =  Frame(None, symbol("base-frame"))
	self.current_frame = self.base_frame
	self.accum = None
	self.paused = True
    

#-------------------------------- EObject types -------------------------------#


nil = EObject(None, "nil")

def nilp(x):
    return (x.type() == "nil")    

class Cons_cell():
    def __init__(self, car, cdr):
	self.car = car
	self.cdr = cdr

def cheap_cons(car, cdr):
    # I call this 'cheap' because it can, in theory, violate the
    # binding algebra that makes Eight work --- but it is much faster
    # than a true Eight cons. You'll see the 'cheap_' prefix in
    # relation to anything that ignores the binding algebra for speed.
    return EObject(Cons_cell(car, cdr), "cons")

def symbol(name):
    return EObject(name, "symbol")

def estring(str):
    return EObject(str, "string")
    

#------------------------------ Binding algebra ------------------------------#

def combine_bindings(a, b):
    # Combine is the heart of the binding algebra! Two bindings, a and
    # b, are given.  Three are returned: a new version of a and b,
    # plus c, a 'combined' binding. The c will contain any
    # symbol<->value pairs that a and b have in common, as well as any
    # pairs the symbol of which appears *only* in a or b. If a symbol
    # appears in *both* a and b, but bound to different values, then
    # those two contradictory symbol<->value pairs will be places in
    # new-a and new-b respectively.
    c = {}
    newa = {}
    newb = {}
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
	    	
    return newa, newb, c
    

def cons(car, cdr):
    ncdr = cdr.copy()
    ncar = car.copy()
    ret = cheap_cons(ncar, ncdr)
    ncar.bindings, ncdr.bindings, ret.bindings = combine_bindings(car.bindings, 
                                                                  cdr.bindings)
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
    # Unioning bindings: two bindings, a and b, become one. Every pair
    # that is in b, and not a 'leaked' symbol, will appear in the new
    # binding. All pairs in a that are not in conflict with b will
    # appear in the new binding.
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
        print "",  stringify(x)

    ret = cheap_car(x).copy()
    ret.bindings = union_bindings(x.bindings, ret.bindings)
    return ret
    

def cdr(x):
    if nilp(x):
        return nil

    if not x.inner.value.__class__ == Cons_cell:
        print "",  stringify(x)
        
    ret = cheap_cdr(x).copy()
    ret.bindings = union_bindings(x.bindings, ret.bindings)
    return ret
    

#----------------------- List functions based on above ------------------------#

def elist(*args):
    ret = nil
    for i in reversed(range(len(args))):
        ret = cons(args[i], ret)	
    return ret
    
def append(x, y):
    # Because Python does not optimise tail-recursion, I will use an
    # iterative solution. Nevertheless, here is what this function
    # would look like, if it were safe:
    #
    # if nilp(x):
    #     return y
    # return cons(car(x), append(cdr(x), y))
    #
    # Alas, it is not to be.
    ret = y
    xlis = []
    while not nilp(x):
        xlis.append(car(x))
        x = cdr(x)
    for i in reversed(range(len(xlis))):
        ret = cons(xlis[i], ret);
    return ret


#------------------------- Symbol binding and lookup --------------------------#


def leaked():
    # leaked just means using the symbol 'LEAKED' as a value. 
    return elist(symbol("LEAKED"))


def leakedp(x):
    if (x and
        x.type() == "cons" and
        cheap_car(x).type() == "symbol" and
        cheap_car(x).inner.value == "LEAKED"):
        return True
    return False
    

def look_up(sym, m):
    # If a symbol is leaked, I should look in the next available scope
    # up the stack. I start with the bindings on the symbol itself:
    if (sym.inner.value in sym.bindings): 
        ret = sym.bindings[sym.inner.value]
    else:
        ret = leaked()
	
    # Then I try the current frame's scope, moving along the frame
    # stack only when the symbol is leaked. If it has no binding at
    # all, ...
    current_frame = m.current_frame

    while leakedp(ret): 
        if sym.inner.value in current_frame.scope:
            ret = current_frame.scope[sym.inner.value]
            current_frame = current_frame.next
        else:
            ret = nil
	
    # ... then I'll look in the base frame. Otherwise, return nil.
    if ret == nil: 
        if sym.inner.value in m.base_frame.scope:
            ret = m.base_frame.scope[sym.inner.value]
	
    return ret
    


def set(sym, val, m):
    # Set just looks up the value; if there is one already, it sets
    # it, respecting closures. Otherwise, it's a global!
    oldval = look_up(sym, m)
    if nilp(oldval):
        m.base_frame.scope[sym.inner.value] = elist(val)
    else:
        oldval.inner.value.car = val
	
    

#--------------------------- Handling Lambda Lists ----------------------------#

def asterixp(x):
    return (x.type() == "cons" and
	    cheap_car(x).type() == "symbol" and
	    cheap_car(x).inner.value == "*")

def atpendp(x):
    return (x.type() == "cons" and
        cheap_car(x).type() == "symbol" and
        cheap_car(x).inner.value == "@")

def elipsisp(x):
    return (x.type() == "symbol" and x.inner.value == "...")
    

def argument_chain(lambda_list, arg_list, chain):
    # I do not appreciate the way this function looks. 
    # Even it' mamma don' love it.

    if nilp(lambda_list):
        return chain

    arg = car(arg_list)

    if asterixp(arg):
        aflag = None
        cflag = "asterpend_continue"
        func = car(cdr(arg))

    elif atpendp(arg):
        aflag = None
        cflag = "atpend_continue"
        func = car(cdr(arg))

    else:
        aflag = "argument"
        cflag = "continue"
        alambda = car(lambda_list)

        if elipsisp(alambda):
            if nilp(arg_list):
                return chain
            else:
                alambda = car(cdr(lambda_list))
                aflag = "e_argument"

        else:
            lambda_list = cdr(lambda_list)
        

        if alambda.type() == "cons":
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
	    
    chain = Operation(chain, elist(lambda_list, cdr(arg_list)), cflag)

    if aflag:
        chain = Operation(chain, name, aflag)

    ret = Operation(chain, func, "evaluate")	

    return ret
    




def clear_elist(x):
    if nilp(x):
        return nil
    return cons(elist(symbol("clear"), car(x)), clear_elist(cdr(x)))
    

def clearp(x):
    return (x.type() == "symbol" and x.inner.value == "clear")



def evaluate_instruction(x, m):
    # This is what the Eight machine will do when it is simply
    # evaluating an EObject; no special flags or nuthin'.

    if x.type() == "builtin":
        x.inner.value(m)
    
    elif x.type() == "symbol":
        ret  = look_up(x, m)

        if nilp(ret):
            signal(elist(estring("That's an unbound symbol, Ma'am:"), x), m)
            return 

        m.accum = car(ret)

    elif x.bindings:
        m.current_frame.scope = union_bindings(
            m.current_frame.scope,
            x.bindings)
        nex = x.copy()
        nex.bindings =  {}
        m.current_frame.next =  Operation(m.current_frame.next,
                                          nex,
                                          "evaluate")
            
    elif x.type() == "cons":
        # If the object's a list, then evaluate the first element, and
        # treat it as a function.

        # Clear is one of only two truly special forms in Eight. If
        # the function is clear, then the machine returns the argument
        # unaltered.
        if clearp(car(x)):
            m.accum = car(cdr(x))
            
        else:
            m.current_frame = Frame(m.current_frame, x)
            m.current_frame.scope = m.current_frame.below.scope
            m.current_frame.next = Operation(m.current_frame.next, cdr(x), "apply")
            m.current_frame.next = Operation(m.current_frame.next, car(x), "evaluate")
		
    else:
        # It's some CRAZY type we've never heard of, amirite? Let's
        # not evaluate it, and hate it because it's different.
        m.accum = x


def machine_step(m):

    if (not m.current_frame.next): 
        if (m.current_frame.below != None):
            m.current_frame = m.current_frame.below
            return
        else :
            m.paused = True
            return    

    instruction = m.current_frame.next
    m.current_frame.next = instruction.next

    if (instruction.flag == "evaluate"):
        evaluate_instruction(instruction.instruction, m)
	    
    elif (instruction.flag == "apply"):
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
                
                
                
#---------------------------------- Continuations -----------------------------#
                
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
        print "\n\nNO BASE SIGNAL HANDLER:", stringify(signal)
    else :
        m.current_frame.next = Operation(m.current_frame.next, 
                                         elist(m.current_frame.signal_handler,
                                               elist(symbol("clear"), signal)),
                                         "evaluate")

#------------------------------ Building in built-ins -------------------------#

def add_fn(m, name, alambda, fn):
    fun = elist(parse(preparse(alambda)),  EObject(fn, "builtin"))
    fun.inner.info["def-name"] = symbol(name)
    m.base_frame.scope[name] = elist(fun)
    

def get_arg(x, m):
    return car(look_up(symbol(x), m))


def add_em_all(m):
    def consb(m):
        cdr =  get_arg("cdr", m)
        if (not (cdr.inner.value.__class__ == Cons_cell) and
            not nilp(cdr)):
            alert("Warning that you should make real elists!")
        m.accum = cons(get_arg("car", m), cdr)

    add_fn(m, "cons", "(car cdr)", consb)

    def carb(m): # Bad for my diet
        x =  get_arg("x", m)
        m.accum = car(x)

    add_fn(m, "car", "(x)", carb)

    def cdrb(m):
        x =  get_arg("x", m)
        m.accum = cdr(x)

    add_fn(m, "cdr", "(x)", cdrb)

    def infob(m):
        x =  get_arg("x", m)
        m.accum =  EObject(x.inner.info, "table")

    add_fn(m, "info", "(x)", infob)
    
    def breakb(m):
        m.paused = True
    add_fn(m, "break", "()", breakb)

    def quoteb(m): # Please don't, I've heard all I wish to
        x =  get_arg("x", m)
        temp = m.current_frame
        m.current_frame = temp.below
        m.accum = enclose(x, m)
        m.current_frame = temp

    add_fn(m, "'", "((clear x))", quoteb)

    def setb(m):
        sym =  get_arg("symbol", m)
        val =  get_arg("value", m)
        set(sym, val, m)
        m.accum = val

    add_fn(m, "set!", "('symbol value)", setb)

    def printb(m):
        val =  get_arg("thing", m)
        print stringify(val),
        m.accum = val
        
    add_fn(m, "print", "(thing)", printb)
    
    def bleak(m): # Ooh, Russian lit
        sym =  get_arg("symbol", m)
        val =  get_arg("expression", m)
        val.bindings[sym.inner.value] = elist(symbol("LEAKED"))
        m.accum = val

    add_fn(m, "leak", "(symbol expression)", bleak)

    def atom_pb(m): # What, lead?
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
    
    def isb(m): # The brother to canby
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
	
    
        
# ---------------------------------- Parsing ----------------------------------#
# This is a bit hacky, yes? TODO: come back and write a nice parser.

import re

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
    

#----------------------------------- Unparsing --------------------------------#


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
        print "I don't know how to stringify that: ", x, ". Sorry, coach."
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
    if not x:
        return ""
    else :
        return ("|-> " + stringify(x.instruction) + ", flagged " + x.flag +
                "\n" + stringify(x.next))
	
    
def stringify_eobject(x):
    typ = x.type()
    if typ == "cons":
        ret = "(" + stringify_elist(x) + ")"
    elif typ == "nil":
        ret = "()"
    elif typ == "string":
        ret = "\"" + x.inner.value + "\""
    elif typ == "builtin":
        ret = "BUILTIN"
    else :
        ret = x.inner.value.__str__()
    return ret
    

def stringify_elist(x):
    if nilp(x.inner.value.cdr):
        return stringify(x.inner.value.car)
    else:
        return (stringify(x.inner.value.car) + " " +
                stringify_elist(x.inner.value.cdr))
