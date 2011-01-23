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
    this.new_scope = new Array();
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

    if (car==nil){
	var ret = cheap_cons(nil, ncdr);
	ret.bindings = ncdr.bindings;
	ncdr.bindings = new Array();
	return ret;
    }

    if (cdr==nil){
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
    if (x==nil){
	return nil;
    }
    return x.in.value.car;
}

function cheap_cdr(x){
    if (x==nil){
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
    if (x==nil){
	return nil;
    }
    var ret = cheap_car(x).copy();
    ret.bindings[sym] = union_bindings(x.bindings, ret.bindings);
    return ret;
}

function cdr(x){
    if (x==nil){
	return nil;
    }
    var ret = cheap_cdr(x).copy();
    ret.bindings[sym] = union_bindings(x.bindings, ret.bindings);
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
	ret = sym.bindings[sym];
    } else {
	ret = leaked();
    }

    var current_frame = m.current_frame;

    while(leakedp(ret)){  // in the current frame (and below)
	ret = current_frame.scope[sym];
	current_frame = current_frame.next;
    }
    if (ret == undefined){ // and in the base frame.
	ret = m.base_frame.scope[sym];
    }

    if (ret == undefined){
	ret = nil;
    }
    return ret;
}


function set(sym, val, m){
    var oldval = look_up(sym, m);
    if (oldval == nil){
	m.base_frame.scope[sym] = list(val);
    } else {
	oldval.in.value.car = val;
    }
}




//-------- machine shit -----------------//


// This will get a bit evil when optional args come into play.
function argument_chain(lambda_list, arg_list, chain){
    var lambda = car(lambda_list);
    if (lambda.type() == cons){
	var name = car(lambda);
	var func = car(cdr(lambda));
    } else {
	var name = lambda;
	var func = null;
    }
    var arg = car(arg_list);
    if (asterixp(arg)){
		   return Operation(
		       Operation(chain,
			   list(lambda_list, arg_list),
			   "asterpend_continue"),
		       arg,
		       "");

    } else if (atpendp(arg)){
		   return Operation(
		       Operation(chain,
			   list(lambda_list, arg_list),
			   "atpend_continue"),
		       arg,
		       "");
    } else {
	return Operation(Operation(chain, name, "argument"),
			 list(func, arg),
			 "");
    }
}

function machine_step(m){
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

    if (instruction.flag == ""){

	if (instruction.type() == "builtin"){
	    instruction.in.value();

	} else if (instruction.type() == "continuation"){
	    alert("No continuations yet, sweetums.");
	    //ignored for now

	} else if (instruction.instruction.type() == "symbol") {
	    m.accum = car(look_up(instruction.instruction));
	    if (m.accum == nil){
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
						 "");

	} else if (car(instruction.instruction).type() == "symbol" &&
		   car(instruction.instruction).in.value == "clear") {
	    m.accum = instruction.instruction;

	// Strings would go here, when the go here.
	} else {

	    // function application will go here.
	}

    }
}