function Interior(thing){
    this.value = thing;
    this.info = new Array();
}

function Object(inner, type){
    this.closing = new Array();
    this.in = new Interior(inner);
    this.in.info['type'] = type;
}

Object.prototype.type = function(){
    return this.in.info['type'];
};

function Frame(below, name){
    this.below = below;
    this.scope = new Array();
    this.signal_handler = null;
    this.trace = name;
}

function Machine(){
    this.base_frame = frame(null, "Base Frame");
    this.current_frame = this.base_frame;
    this.accum = null;
}

//--------------------------------- coo.

nil = new Object(null, "nil");

function Cons_cell(car, cdr){
    this.car = car;
    this.cdr = cdr;
}

function cons_pair(car, cdr){
    return new Object(new Cons_cell(car, cdr), "cons");
}

function symbol(name){
    return new Object(name, "symbol");
}

function cheap_list() {
    var ret = nil;
    for (var i=arguments.length-1; i>=0; i--){
	ret = cons_pair(arguments[i], ret);
    }
    return ret;
}

