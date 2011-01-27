// ------------------------------ Parsing ----------------------------//
// This is a bit hacky, yes? TODO: come back and write a nice parser.

function preparse(str){
    str = str.split(/\"/);
    var astr = new Array();
    for (a in str){
     	if (a % 2 == 0){
	    astr = astr.concat(tokenize(str[a]));
     	} else {
	    astr.push("\"" + str[a]);
	}
	//for (a in astr){print(astr[a]);}
    }
    return astr.reverse();
}

function tokenize(str){
    str = str.replace(/\#.*\n/g, " ");

    str = str.replace(/\'\(/g, " '[ ");
    str = str.replace(/\@\(/g, " @[ ");
    str = str.replace(/\*\(/g, " *[ ");
    str = str.replace(/\,\(/g, " ,[ ");

    str = str.replace(/\(/g, " ( ");
    str = str.replace(/\)/g, " ) ");

    str = str.replace(/\'\[/g, " '( ");
    str = str.replace(/\@\[/g, " @( ");
    str = str.replace(/\*\[/g, " *( ");
    str = str.replace(/\,\[/g, " ,( ");

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
    } else if (token[0] == "'"){
	tokens.push(token.substr(1, token.length));
	return list(symbol("'"), parse(tokens));
    } else if (token[0] == "@"){
	tokens.push(token.substr(1, token.length));
	return list(symbol("@"), parse(tokens));
    } else if (token[0] == "*"){
	tokens.push(token.substr(1, token.length));
	return list(symbol("*"), parse(tokens));
    } else if (token[0] == ","){
	tokens.push(token.substr(1, token.length));
	return list(symbol(","), parse(tokens));
    }
    // else if (token[0] == "$"){
    // return new Character(token[1]);
    // }
    else if (token[0] == "\""){
	return estring(token.substr(1, token.length));
    }
    // else if (token.match(/^[\d\-\.]+$/)){
    // return new Number(parseFloat(token));
    // }
    else {
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
	alert("I don't know how to stringigy that.");
	return "STRINGIFY ERR";
    }
}

function stringify_frame(x){
    if (!x) {
	return "";
    } else {
	return (x.trace + ", scoped as " + x.scope + "performing:\n" +
		stringify(x.next) + "\n\n" +
		stringify_operation(x.next));
    }
}


function stringify_operation(x){
    if (!x) {
	return "";
    } else {
	return (stringify(x.instruction) + ", flagged " + x.flag +
		"\n |-> " + stringify_operation(x.next));
    }
}

function stringify_eobject(x){
    var type = x.type();
    if (type == "cons") {
	return "(" + stringify_list(x) + ")";
    } else if (type == "nil"){
	return "()";
    } else if (type == "string"){
	return "\"" + x.in.value + "\"";
    } else {
	return x.in.value;
    }
}

function stringify_list(x){
    if (x.in.value.cdr.type() == "nil"){
	return stringify(x.in.value.car);
    } else {
	return stringify(x.in.value.car) + " " + stringify_list(x.in.value.cdr);
    }
}