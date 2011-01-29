function Interior(thing){
    this.value = thing;
    this.info = new Array();
}

function EObject(inner, type){
    this.bindings = new Array();
    this.in = new Interior(inner);
    this.in.info['type'] = type;
}

EObject.prototype.type = function(){
    return this.in.info['type'];
};

EObject.prototype.copy = function() {
    var ret = new EObject(this.in.value, this.type());
    ret.in.info = this.in.info;
    ret.bindings = this.bindings;
    return ret;
};

function Operation(next, instruction, flag){
    this.next = next;
    this.instruction = instruction;
    this.flag = flag;
}

function Frame(below, name){
    this.below = below;
    this.next = nil;
    this.scope = new Array();
    this.rib = new Array();
    this.signal_handler = null;
    this.trace = name;
}


function Machine(){
    this.base_frame = new Frame(null, symbol("base-frame"));
    this.current_frame = this.base_frame;
    this.accum = null;
}

function copy_scope(x){
    var ret = new Array();
    for (var a in x){
	ret[a] = list(car(x[a]));
    }
    return ret;
}

//--------------------------------- coo.

nil = new EObject(null, "nil");

function nilp(x){
    return (x.type() == "nil");
}

function Cons_cell(car, cdr){
    this.car = car;
    this.cdr = cdr;
}

function cheap_cons(car, cdr){
    return new EObject(new Cons_cell(car, cdr), "cons");
}

function symbol(name){
    return new EObject(name, "symbol");
}

function estring(str){
    return new EObject(str, "string");
}

//------------- Binding algebra ----------------------//

function empty_binding_p(binding) {
    for (var x in binding){
	return false;
    }
    return true;
}


function combine_bindings(a, b, newa, newb){
    var c = new Array();
    for(var sym in a){
	if (sym in b){
	    if(a[sym] == b[sym]){ // do I mean equal?
		c[sym] = a[sym];
	    } else {
		newa[sym] = a[sym];
		newb[sym] = a[sym];
	    }
	} else {
	    c[sym] = a[sym];
	}
    }
    for (var sym in b){
	if (!(sym in a)){
	    c[sym] = b[sym];
	}
    }
    return c;
}

function cons(car, cdr){
    var ncar = car.copy();
    var ncdr = cdr.copy();

    if (nilp(car)){
	var ret = cheap_cons(nil, ncdr);
	ret.bindings = ncdr.bindings;
	ncdr.bindings = new Array();
	return ret;
    }

    if (nilp(cdr)){
	var ret = cheap_cons(ncar, nil);
	ret.bindings = ncar.bindings;
	ncar.bindings = new Array();
	return ret;
    }

    ncar.bindings = new Array();
    ncdr.bindings = new Array();
    var ret = cheap_cons(ncar, ncdr);
    ret.bindings = combine_bindings(car.bindings, cdr.bindings,
				    ncar.bindings, ncdr.bindings);
    return ret;
}

function cheap_car(x){
    if (nilp(x)){
	return nil;
    }
    return x.in.value.car;
}

function cheap_cdr(x){
    if (nilp(x)){
	return nil;
    }
    return x.in.value.cdr;
}

function union_bindings(a, b){
    var ret = new Array();
    for (var sym in a){
	ret[sym] = a[sym];
    }
    for (var sym in b){
	if (!leakedp(b[sym])){
	    ret[sym] = b[sym];
	}
    }
    return ret;
}

function car(x){
    if (!(x.in.value instanceof Cons_cell)) {
	alert("Car of a thing not consed!");
    }

    if (nilp(x)){
	return nil;
    }
    var ret = cheap_car(x).copy();
    ret.bindings = union_bindings(x.bindings, ret.bindings);
    return ret;
}

function cdr(x){
    if (!(x.in.value instanceof Cons_cell)) {
	alert("Cdr of a thing not consed!");
    }
    if (nilp(x)){
	return nil;
    }
    var ret = cheap_cdr(x).copy();
    ret.bindings = union_bindings(x.bindings, ret.bindings);
    return ret;
}

//---------------- OK. -------------------//

function cheap_list() {
    var ret = nil;
    for (var i=arguments.length-1; i>=0; i--){
	ret = cons_pair(arguments[i], ret);
    }
    return ret;
}

function list() {
    var ret = nil;
    for (var i=arguments.length-1; i>=0; i--){
	ret = cons(arguments[i], ret);
    }
    return ret;
}

function append(x, y){
    // Currently possible to blow the stack.
    if (nilp(x)){
	return y;
    }
    return cons(car(x), append(cdr(x), y));
}


//--------------- symbol binding shit --------------//


function leaked(){
    return list(symbol("leak"));
}

function leakedp(x){
    if (x &&
	x.type() == "cons" &&
	cheap_car(x).type() == "symbol" &&
	cheap_car(x).in.value == "leak"){
	return true;
    }
    return false;
}

function look_up(sym, m){
    var ret;

    if (sym.in.value in sym.bindings){ // check in the local closing
	ret = sym.bindings[sym.in.value];
    } else {
	ret = leaked();
    }

    var current_frame = m.current_frame;

    while(leakedp(ret)){  // in the current frame (and below)
	ret = current_frame.scope[sym.in.value];
	current_frame = current_frame.next;
    }

    if (ret == undefined){ // and in the base frame.
	ret = m.base_frame.scope[sym.in.value];
    }

    if (ret == undefined){
	ret = nil;
    }
    return ret;
}


function set(sym, val, m){
    var oldval = look_up(sym, m);
    if (nilp(oldval)){
	m.base_frame.scope[sym.in.value] = list(val);
    } else {
	oldval.in.value.car = val;
    }
}




//-------- machine shit -----------------//

function asterixp(x){
    if (x.type() == "cons" &&
	cheap_car(x).type() == "symbol" &&
	cheap_car(x).in.value == "*"){
	return true;
    }
    return false;
}

function atpendp(x){
    if (x.type() == "cons" &&
	cheap_car(x).type() == "symbol" &&
	cheap_car(x).in.value == "@"){
	return true;
    }
    return false;
}

function elipsisp(x){
    return (x.type()=="symbol" &&
	    x.in.value == "...");
}

// Also, DESPERATELY need to add elipsis args.
function argument_chain(lambda_list, arg_list, chain){

    if (lambda_list.type() == "nil"){
	return chain;
    }

    var last;
    var ret;
    var arg = car(arg_list);
    if (asterixp(arg)){
	last = new Operation(chain,
			     list(lambda_list, cdr(arg_list)),
			     "asterpend_continue");
	ret = new Operation(last, car(cdr(arg)), "evaluate");
    } else if (atpendp(arg)){
	last = new Operation(chain, list(lambda_list, cdr(arg_list)), "atpend_continue");
	ret = new Operation(last, car(cdr(arg)), "evaluate");

    } else {
	var name, func, lambda = car(lambda_list);
	var flag = "argument";

	if (elipsisp(lambda)) {
	    if (nilp(arg_list)) {
		return chain;
	    }
	    lambda = car(cdr(lambda_list));
	    lambda_list = cons(nil, lambda_list);
	    flag = "e_argument";
	}

	if (lambda.type() == "cons"){
	    name = car(cdr(lambda));
	    func  = list(car(lambda), arg);
	} else {
	    name = lambda;
	    func = arg;
	}
	last = new Operation(chain, list(cdr(lambda_list), cdr(arg_list)), "continue");
	ret = new Operation(new Operation(last, name, flag), func, "evaluate");
    }
    return ret;
}

function clear_list(x){
    if (nilp(x)){
	return nil;
    }
    return cons(list(symbol("clear"), car(x)), clear_list(cdr(x)));
}

function machine_step(m){
    // this is the heart of the eight machine;
    // and if an imperative js function this long doesn't make you
    // at least a little squeamish, you're not human.

    var instruction = m.current_frame.next;

    if (instruction == null) {   // this block returns
	// The current frame contains no further instructions.
	if (m.current_frame.below != null){
	    // Move on to the next frame.
	    m.current_frame = m.current_frame.below;
	    return 0;
	} else {
	    // The frame below is the base frame, so we must be done.
	    return 1;
	}
    }

    m.current_frame.next= instruction.next;

    if (instruction.flag == "evaluate"){
	if (instruction.instruction.type() == "builtin"){
	    instruction.instruction.in.value(m);

	} else if (instruction.instruction.type() == "symbol") {
	    //alert("here");
	    //alert(stringify(car(look_up(instruction.instruction, m))));
	    var ret  = look_up(instruction.instruction, m);
	    if (nilp(ret)){
		//signal
		alert("thass no' a symbol, lassie: " + stringify(instruction.instruction));
	    }
	    m.accum = car(ret);

	} else if (!empty_binding_p(instruction.instruction.bindings)){
	    m.current_frame.scope = union_bindings(
					m.current_frame.scope,
					instruction.instruction.bindings);
	    var nex = instruction.instruction.copy();
	    nex.bindings = new Array();
	    m.current_frame.next = new Operation(m.current_frame.next,
						 nex,
						 "evaluate");

	} else if (instruction.instruction.type() == "cons") {

	    if (car(instruction.instruction).type() == "symbol" &&
		car(instruction.instruction).in.value == "clear") {
		m.accum = car(cdr(instruction.instruction));

	    } else {
		var name = instruction.instruction;
		m.current_frame = new Frame(m.current_frame, name);
		m.current_frame.scope = m.current_frame.below.scope;
		m.current_frame.next = new Operation(
		new Operation(
		    m.current_frame.next,
		    cdr(instruction.instruction),
		    "apply"),
		car(instruction.instruction),
		"evaluate");
	    }

	} else {
	    // It's some CRAZY type, amirite? Let's not evaluate it,
	    // and hate it because it's different.
	    m.accum = instruction.instruction;
	}
    } else {
	// So we must be dealing with a fancy flag.
	if (instruction.flag == "apply"){
	    m.current_frame.next = new Operation(m.current_frame.next,
						 cdr(m.accum),
						 "do");
	    m.current_frame.next = argument_chain(car(m.accum),
						  instruction.instruction,
						  m.current_frame.next);

	} else if (instruction.flag == "continue"){
	    m.current_frame.next = argument_chain(car(instruction.instruction),
						  car(cdr(instruction.instruction)),
						  m.current_frame.next);

	} else if (instruction.flag == "asterpend_continue"){
	    m.current_frame.next = argument_chain(
		car(instruction.instruction),
		append(m.accum, car(cdr(instruction.instruction))),
		m.current_frame.next);

	} else if (instruction.flag == "atpend_continue"){
	    m.current_frame.next = argument_chain(
		car(instruction.instruction),
		append(clear_list(m.accum), car(cdr(instruction.instruction))),
		m.current_frame.next);

	} else if (instruction.flag == "argument"){
	    m.current_frame.rib[instruction.instruction.in.value] = list(m.accum);

	} else if (instruction.flag == "e_argument"){
	    var sym = instruction.instruction.in.value;
	    if (sym in m.current_frame.rib){
		var so_far = m.current_frame.rib[sym];
		so_far = append(car(so_far), list(m.accum));
		m.current_frame.rib[sym] = list(so_far);
	    } else {
		m.current_frame.rib[sym] = list(list(m.accum));
	    }

	} else if (instruction.flag == "do") {
	    m.current_frame.scope = m.current_frame.rib;
	    var temp = m.current_frame.next;
	    var chain = m.current_frame;
	    while(!nilp(instruction.instruction)){
		chain.next = new Operation(m.current_frame.next,
					   car(instruction.instruction),
					   "evaluate");
		chain = chain.next;
		instruction.instruction = cdr(instruction.instruction);
	    }
	    chain.next = temp;
	}
    }
    return 0;
}


function copy_frame(x) {
    var below;
    if (x.below){
	below = copy_frame(x.below);
    } else {
	return x;
    }

    var ret = new Frame(below, x.name);
    ret.next = x.next;
    ret.scope = copy_scope(x.scope);
    ret.rib = copy_scope(x.rib);
    ret.signal_handler = x.signal_handler;
    return ret;
};


function build_continuation(m){
    var q = new Machine();
    q.base_frame = m.base_frame;
    q.current_frame = copy_frame(m.current_frame);
    var ret = new EObject(q, "continuation");

}

function signal(m){

}


//--------------- building it in -----------------//

function add_function(m, name, lambda, fn){
    var fun = list(parse(preparse(lambda)), new EObject(fn, "builtin"));
    fun.in.info["function-name"] = symbol(name);
    m.base_frame.scope[name] = list(fun);
}

function get_arg(x, m){
    return car(look_up(symbol(x), m));
}

function add_em_all(m){
    add_function(m, "cons", "(car cdr)", function(m){
		     var cdr =  get_arg("cdr", m);
		     if (cdr.type() != "cons" && !nilp(cdr)){
			 alert("warning that you should make real lists");
		     }
		     m.accum = cons(get_arg("car", m), cdr);
		 });

    add_function(m, "car", "(x)", function(m){
		     var x =  get_arg("x", m);
		     m.accum = car(x);
		 });

    add_function(m, "cdr", "(x)", function(m){
		     var x =  get_arg("x", m);
		     m.accum = cdr(x);
		 });

    add_function(m, "info", "(x)", function(m){
		     var x =  get_arg("x", m);
		     m.accum = new EObject(x.in.info, "table");
		 });

    add_function(m, "'", "((clear x))", function(m){
		     var x =  get_arg("x", m);
		     var temp = m.current_frame;
		     m.current_frame = temp.below;
		     m.accum = enclose(x, m);
		     m.current_frame = temp;
		 });

    add_function(m, "set!", "('symbol value)", function(m){
		     var sym =  get_arg("symbol", m);
		     var val =  get_arg("value", m);
		     set(sym, val, m);
		     m.accum = val;
		 });

    add_function(m, "leak", "('symbol expression)", function(m){
		     var sym =  get_arg("symbol", m);
		     var val =  get_arg("expression", m);
		     val.bindings[sym.in.value] = list(symbol("leak"));
		     m.accum = set(sym, val, m);
		 });

    add_function(m, "atom-p", "(x)", function(m){
		     var x =  get_arg("x", m);
		     if (x.type() == "cons"){
			 m.accum = nil;
		     } else {
			 m.accum = symbol("t");
		     }
		 });

    add_function(m, "oif", "(test 'then 'else)", function(m){
		     var test =  get_arg("test", m);
		     if(!nilp(test)){
			 m.current_frame.next = new Operation(m.current_frame.next,
							      get_arg("then", m),
							      "evaluate");
		     } else {
			 m.current_frame.next = new Operation(m.current_frame.next,
							      get_arg("else", m),
							      "evaluate");
		     }
		 });

    add_function(m, "is", "(a b)", function(m){
		     var a =  get_arg("a", m);
		     var b =  get_arg("b", m);
		     if (a == b ||
			 (a.type() == b.type() &&
			 a.in.value === b.in.value)) {
			 m.accum = symbol("t");
		     } else {
			 m.accum = nil;
		     }
		 });
}


function enclose(code, m){
    code.bindings = new Array();
    find_free_variables(code, code.bindings, m);
    return code;
}

function free_var_p(sym, bindings){
    return (sym.type() == "symbol" && !(sym.in.value in bindings));
}

function find_free_variables(code, bindings, m){
    if (free_var_p(code, bindings)){
	var a = look_up(code, m);
	if (!nilp(a)){
	    bindings[code.in.value] = a;
	}
    } else if (code.type() == "cons") {
	find_free_variables(car(code), bindings, m);
	find_free_variables(cdr(code), bindings, m);
    }
}

function init_machine(){
    var m = new Machine();
    m.base_frame.scope["q"] = list(symbol("sq"));
    m.base_frame.scope["r"] = list(symbol("sr"));
    m.base_frame.scope["s"] = list(symbol("ss"));
    add_em_all(m);
    return m;
}

