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
    this.base_frame = new Frame(null, "Base Frame");
    this.current_frame = this.base_frame;
    this.accum = null;
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
    if (nilp(x)){
	return nil;
    }
    var ret = cheap_car(x).copy();
    ret.bindings = union_bindings(x.bindings, ret.bindings);
    return ret;
}

function cdr(x){
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
    alert(stringify(x));
    alert(stringify(y));
    if (nilp(x)){
	return y;
    }
    var b = append(cdr(x), y);
    alert(stringify(b));
    return cons(car(x), b);
}
//--------------- symbol binding shit --------------//


function leaked(){
    return list(new EObject("leak", "leak"));
}

function leakedp(x){
    if (x &&
	x.type() == "cons" &&
	cheap_car(x).type() == "leak"){
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
	m.base_frame.scope[sym] = list(val);
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

// Also, DESPERATELY need to add elipsis args.
function argument_chain(lambda_list, arg_list, chain){
    alert(stringify(lambda_list));
    alert(stringify(arg_list));
    alert(stringify(chain));

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
	ret = new Operation(last, arg, "evaluate");
    } else {

	var name, func, lambda = car(lambda_list);
	if (lambda.type() == "cons"){
	    name = car(cdr(lambda));
	    func  = list(car(lambda), arg);
	} else {
	    name = lambda;
	    func = arg;
	}
	last = new Operation(chain, list(cdr(lambda_list), cdr(arg_list)), "continue");
	ret = new Operation(new Operation(last, name, "argument"), func, "evaluate");
    }
    return ret;
}

function clear_list(list){

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
	    instruction.in.value();

	} else if (instruction.instruction.type() == "continuation"){
	    alert("No continuations yet, sweetums.");
	    //ignored for now

	} else if (instruction.instruction.type() == "symbol") {
	    //alert("here");
	    //alert(stringify(car(look_up(instruction.instruction, m))));
	    m.accum = car(look_up(instruction.instruction, m));
	    if (nilp(m.accum)){
		//signal
		alert("thass no' a symbol, lassie");
	    }

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
		m.current_frame = new Frame(m.current_frame, "funcall");
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
	    append(m.accum, car(cdr(instruction.instruction)));
	    m.current_frame.next = argument_chain(
		car(instruction.instruction),
		append(m.accum, car(cdr(instruction.instruction))),
		m.current_frame.next);

	} else if (instruction.flag == "atpend_continue"){

	} else if (instruction.flag == "argument"){
	    m.current_frame.rib[instruction.instruction.in.value] = list(m.accum);

	} else if (instruction.flag == "do") {
	    m.current_frame.scope = m.current_frame.rib;
	    m.current_frame.next = new Operation(m.current_frame.next,
						 car(instruction.instruction),
						 "evaluate");
	}
    }
    return 0;
}


