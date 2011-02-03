var eight = new function(){

    function Interior(thing){
	this.value = thing;
	this.info = new Array();
    }

    function EObject(inner, type){
	this.bindings = new Array();
	this.inner = new Interior(inner);
	this.inner.info['type'] = type;
    }

    EObject.prototype.type = function(){
	return this.inner.info['type'];
    };

    EObject.prototype.copy = function() {
	var ret = new EObject(this.inner.value, this.type());
	ret.inner.info = this.inner.info;
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
	this.paused = true;
    }

    function copy_scope(x){
	var ret = new Array();
	for (var a in x){
	    ret[a] = list(car(x[a]));
	}
	return ret;
    }

    //--------------------------------- coo.

    var nil = new EObject(null, "nil");

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
		    newb[sym] = b[sym];
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
	return x.inner.value.car;
    }

    function cheap_cdr(x){
	if (nilp(x)){
	    return nil;
	}
	return x.inner.value.cdr;
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

	if (!(x.inner.value instanceof Cons_cell)) {
	    alert("Car of a thing not consed!" + stringify(x));
	}

	var ret = cheap_car(x).copy();
	ret.bindings = union_bindings(x.bindings, ret.bindings);
	return ret;
    }

    function cdr(x){
	if (nilp(x)){
	    return nil;
	}

	if (!(x.inner.value instanceof Cons_cell)) {
	    alert("Cdr of a thing not consed!");
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
	    cheap_car(x).inner.value == "leak"){
	    return true;
	}
	return false;
    }

    function look_up(sym, m){
	var ret;

	if (sym.inner.value in sym.bindings){ // check in the local closing
	    ret = sym.bindings[sym.inner.value];
	} else {
	    ret = leaked();
	}

	var current_frame = m.current_frame;

	while(leakedp(ret)){  // in the current frame (and below)
	    ret = current_frame.scope[sym.inner.value];
	    current_frame = current_frame.next;
	}

	if (ret == undefined){ // and in the base frame.
	    ret = m.base_frame.scope[sym.inner.value];
	}

	if (ret == undefined){
	    ret = nil;
	}
	return ret;
    }


    function set(sym, val, m){
	var oldval = look_up(sym, m);
	if (nilp(oldval)){
	    m.base_frame.scope[sym.inner.value] = list(val);
	} else {
	    oldval.inner.value.car = val;
	}
    }




    //-------- machine shit -----------------//

    function asterixp(x){
	if (x.type() == "cons" &&
	    cheap_car(x).type() == "symbol" &&
	    cheap_car(x).inner.value == "*"){
	    return true;
	}
	return false;
    }

    function atpendp(x){
	if (x.type() == "cons" &&
	    cheap_car(x).type() == "symbol" &&
	    cheap_car(x).inner.value == "@"){
	    return true;
	}
	return false;
    }

    function elipsisp(x){
	return (x.type()=="symbol" &&
		x.inner.value == "...");
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
	    last = new Operation(chain,
				 list(lambda_list, cdr(arg_list)),
				 "atpend_continue");
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
		if (arg.type() == "cons" &&
		    car(arg).type() == "symbol" &&
		    car(arg).inner.value == ","){

		    func = car(cdr(arg));
		} else {
		    func  = list(car(lambda), arg);
		}
	    } else {
		name = lambda;
		func = arg;
	    }
	    last = new Operation(chain,
				 list(cdr(lambda_list), cdr(arg_list)),
				 "continue");
	    ret = new Operation(new Operation(last, name, flag),
				func,
				"evaluate");
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
		return;
	    } else {
		// The frame below is the base frame, so we must be done.
		m.paused = true;
		return;
	    }
	}

	m.current_frame.next = instruction.next;

	if (instruction.flag == "evaluate"){
	    if (instruction.instruction.type() == "builtin"){
		instruction.instruction.inner.value(m);

	    } else if (instruction.instruction.type() == "symbol") {
		//alert("here");
		//alert(stringify(car(look_up(instruction.instruction, m))));
		var ret  = look_up(instruction.instruction, m);
		if (nilp(ret)){
		    //signal
		    alert("thass no' a symbol, lassie: " +
			  stringify(instruction.instruction));
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
		    car(instruction.instruction).inner.value == "clear") {
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
		m.current_frame.next = argument_chain(
		    car(instruction.instruction),
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
		    append(clear_list(m.accum),
			   car(cdr(instruction.instruction))),
		    m.current_frame.next);

	    } else if (instruction.flag == "argument"){
		m.current_frame.rib[instruction.instruction.inner.value] = list(m.accum);

	    } else if (instruction.flag == "e_argument"){
		var sym = instruction.instruction.inner.value;
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
	var c_frame = copy_frame(m.current_frame);
	var f = function(m){
	    var val = get_arg("val", m);
	    m.current_frame = c_frame;
	    m.accum = val;
	};
	return list(list(symbol("val")), new EObject(f, "builtin"));
    }

    function signal(m, message){
	var sig = list(build_continuation(m), message);
	toss_signal(sig, m);
    }

    function toss_signal(signal, m){
	m.current_frame = m.current_frame.below;
	while(!m.current_frame.signal_handler && m.current_frame.below){
	    m.current_frame = m.current_frame.below;
	}
	if (!m.current_frame.signal_handler && !m.current_frame.below){
		alert("no goddamn signal handler!");
	} else {
	    m.current_frame.next = new Operation(m.current_frame.next, list(m.current_frame.signal_handler, list(symbol("clear"), signal)), "evaluate");
	}
    }


    //--------------- building it in -----------------//

    function add_function(m, name, lambda, fn){
	var fun = list(parse(preparse(lambda)), new EObject(fn, "builtin"));
	fun.inner.info["function-name"] = symbol(name);
	m.base_frame.scope[name] = list(fun);
    }

    function get_arg(x, m){
	return car(look_up(symbol(x), m));
    }

    function add_em_all(m){
	add_function(m, "cons", "(car cdr)", function(m){
			 var cdr =  get_arg("cdr", m);
			 if (!(cdr.inner.value instanceof Cons_cell) &&
			     !nilp(cdr)){
			     alert("Warning that you should make real lists!");
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
			 m.accum = new EObject(x.inner.info, "table");
		     });

	add_function(m, "break", "()", function(m){
			 m.paused = true;
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

	add_function(m, "print", "(thing)", function(m){
			 var val =  get_arg("thing", m);
			 output_string(stringify(val));
			 m.accum = val;
		     });

	add_function(m, "leak", "(symbol expression)", function(m){
			 var sym =  get_arg("symbol", m);
			 var val =  get_arg("expression", m);
			 val.bindings[sym.inner.value] = list(symbol("leak"));
			 m.accum = val;
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
			     m.current_frame.next = new Operation(
							m.current_frame.next,
							get_arg("then", m),
							"evaluate");
			 } else {
			     m.current_frame.next = new Operation(
							m.current_frame.next,
							get_arg("else", m),
							"evaluate");
			 }
		     });

	add_function(m, "is", "(a b)", function(m){
			 var a =  get_arg("a", m);
			 var b =  get_arg("b", m);
			 if (a == b ||
			     (a.type() == b.type() &&
			      a.inner.value === b.inner.value)) {
			     m.accum = symbol("t");
			 } else {
			     m.accum = nil;
			 }
		     });

	add_function(m, "plus", "(a b)", function(m){
			 var a =  get_arg("a", m);
			 var b =  get_arg("b", m);
			     m.accum = new EObject(a.inner.value +
						   b.inner.value,
						   "number");
		     });

	add_function(m, "call/cc", "('function)", function(m){
			 var a =  get_arg("function", m);
			 m.current_frame.next = new Operation(
			     m.current_frame.next,
			     list(a, list(symbol("clear"),
					  build_continuation(m))),
			     "evaluate");
		     });

	add_function(m, "signal", "(message)", function(m){
			 var a =  get_arg("message", m);
			 signal(m, a);
		     });
	add_function(m, "handle-signals", "('function ... 'rest)", function(m){
			 var a =  get_arg("function", m);
			 m.current_frame.signal_handler = a;
			 m.current_frame.next = new Operation(
			     m.current_frame.next,
			     get_arg("rest", m),
			     "do");
		    });
	add_function(m, "base-signal-handler", "('function)", function(m){
			 var a =  get_arg("function", m);
			 m.base_frame.signal_handler = a;
			 m.accum = a;
		     });

	add_function(m, "unhandle-signal", "(signal)", function(m){
			 var a =  get_arg("signal", m);
			 toss_signal(a, m);
			 toss_signal(a, m);
		    });

    }


    function enclose(code, m){
	code.bindings = new Array();
	find_free_variables(code, code.bindings, m);
	return code;
    }

    function free_var_p(sym, bindings){
	return (sym.type() == "symbol" && !(sym.inner.value in bindings));
    }

    function find_free_variables(code, bindings, m){
	if (free_var_p(code, bindings)){
	    var a = look_up(code, m);
	    if (!nilp(a)){
		bindings[code.inner.value] = a;
	    }
	} else if (code.type() == "cons") {
	    find_free_variables(car(code), bindings, m);
	    find_free_variables(cdr(code), bindings, m);
	}
    }

    function init_machine(str){
	var m = new Machine();
	m.base_frame.scope["q"] = list(symbol("sq"));
	m.base_frame.scope["r"] = list(symbol("sr"));
	m.base_frame.scope["s"] = list(symbol("ss"));
	add_em_all(m);
	return m;
    }


    function execute(str, m){
	var op = new Operation(null, ultraparse(str), "evaluate");
	m.current_frame = new Frame(m.base_frame, symbol("initial"));
	m.current_frame.next = op;
	m.paused = false;

	while(m.paused == false){
	    machine_step(m);
	}

	return m.accum;
    }

    function load(str, m){
	var lamda = ultraparse(str);
	var op = new Operation(null, lamda, "evaluate");
	m.current_frame = new Frame(m.base_frame, symbol("initial"));
	m.current_frame.next = op;
	m.paused = false;
    }

    function step(m){
	m.paused = false;
	machine_step(m);
    }

    function continu(m){
	m.paused = false;
	while(m.paused == false){
	    machine_step(m);
	}
    }


    // ------------------------------ Parsing ----------------------------//
    // This is a bit hacky, yes? TODO: come back and write a nice parser.

    function ultraparse(str){
	return parse(preparse("((clear (() "+str+")))"));
    }

    function preparse (str){
	str = str.split(/\"/);
	var astr = new Array();
	for (a in str){
     	    if (a % 2 == 0){
		astr = astr.concat(tokenize(str[a]));
     	    } else {
		astr.push("\"" + str[a] + "\"");
	    }
	}
	return astr.reverse();
    }

    function tokenize(str){
	str = str.replace(/\#.*\n/g, " ");

	str = str.replace(/\(/g, "( ");
	str = str.replace(/[^*',@]\(/g, " (");

	str = str.split(/\s+/);
	var out = new Array();
	var i = 0;
	for (token in str) {
	    if (str[token]){
		out[i] = str[token];
		i++;
	    }
	}
	return out;
    }


    function parse(tokens) {
	var token = tokens.pop();
	if (token == "("){
	    return parse_list(tokens);

	} else if (token[0] == "'" && token.length > 1){
	    tokens.push(token.substr(1, token.length));
	    return list(symbol("'"), parse(tokens));

	} else if (token[0] == "@" && token.length > 1){
	    tokens.push(token.substr(1, token.length));
	    return list(symbol("@"), parse(tokens));

	} else if (token[0] == "*" && token.length > 1){
	    tokens.push(token.substr(1, token.length));
	    return list(symbol("*"), parse(tokens));

	} else if (token[0] == "," && token.length > 1){
	    tokens.push(token.substr(1, token.length));
	    return list(symbol(","), parse(tokens));

	} else if (token[0] == "\""){
	    return estring(token.substr(1, token.length-1));

	} else if (token.match(/^\-?\d+\.?\d*?$/)){
	    return new EObject(parseFloat(token), "number");

	} else {
	    return symbol(token);
	}
    }

    function parse_list(tokens) {
	var lis = new Array();
	while(tokens[tokens.length-1] != ")"){
	    lis.push(parse(tokens));
	}
	tokens.pop();
	return list.apply(this, lis);
    }

    //------------------------- Unparsing -------------------------//

    function stringify(x){
	if (!x){
	    return "NULL";
	} else if (x instanceof EObject){
	    return stringify_eobject(x);
	} else if (x instanceof Operation){
	    return stringify_operation(x);
	} else if (x instanceof Frame){
	    return stringify_frame(x);
	} else {
	    alert("I don't know how to stringiy that.");
	    return "STRINGIFY ERR";
	}
    }

    function stringify_frame(x){
	if (!x) {
	    return "";
	} else {
	    return (stringify(x.trace) +
		    "\n scoped as " + stringify_scope(x.scope) +
		    "\n ribbed as " + stringify_scope(x.rib) +
		    "\n performing:\n" +
		    stringify_operation(x.next)) +
		"\n\n" + stringify_frame(x.below);
	}
    }

    function stringify_scope(x){
	var ret = "";
	for (var a in x){
	    ret += "\n" + a + " -> " + stringify(x[a]);
	}
	return ret;
    }

    function stringify_operation(x){
	if (x == null) {
	    return "";
	} else {
	    return ("|-> " + stringify(x.instruction) + ", flagged " + x.flag +
		    "\n" + stringify_operation(x.next));
	}
    }

    function stringify_eobject(x){
	var type = x.type();
	var ret;
	if (type == "cons") {
	    ret = "(" + stringify_list(x) + ")";
	} else if (type == "nil"){
	    ret = "()";
	} else if (type == "string"){
	    ret = "\"" + x.inner.value + "\"";
	} else if (type == "builtin"){
	    ret = "BUILTIN";
	} else {
	    ret = x.inner.value + "";
	}
	//    ret += "[" + stringify_scope(x.bindings) + "]";
	return ret;
    }

    function stringify_list(x){
	if (x.inner.value.cdr.type() == "nil"){
	    return stringify(x.inner.value.car);
	} else {
	    return (stringify(x.inner.value.car) + " " +
		    stringify_list(x.inner.value.cdr));
	}
    }



    //------------------ Externs -------------------------//

    this.continu = continu;
    this.step = step;
    this.load = load;
    this.execute = execute;
    this.init_machine = init_machine;
    this.stringify = stringify;
    this.parse = parse;
    this.preparse = preparse;
    this.ultraparse = ultraparse;
}();