function interior(thing){
    this.value = thing;
    this.info = new Array();
}

function object(inner, type){
    this.closing = new Array();
    this.in = interior(inner);
    this.in.info['type'] = type;
}

function frame(below, name){
    this.below = below;
    this.scope = new Array();
    this.signal_handler = null;
    this.trace = name;
}

function machine(){
    this.base_frame = frame(null, "Base Frame");
    this.current_frame = this.base_frame;
    this.accum = null;
}

//--------------------------------- coo.

NIL = object(null, "NIL");

function cons_cell(car, cdr){
    this.car = car;
    this.cdr = cdr;
}

function cons_pair(car, cdr){
    object(cons_cell(car, cdr), "cons");
}

function symbol(name){
    return object(name, "symbol");
}

