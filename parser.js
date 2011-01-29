// ------------------------------ Parsing ----------------------------//
// This is a bit hacky, yes? TODO: come back and write a nice parser.

function ultraparse(str){
    return parse(preparse("((clear (() "+str+")))"));
}
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
    else if (token.match(/^\-?\d+\.?\d*?$/)){
	return new EObject(parseFloat(token), "number");
    }
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
	alert("I don't know how to stringiy that.");
	return "STRINGIFY ERR";
    }
}

function stringify_frame(x){
    if (!x.below) {
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
	ret = "\"" + x.in.value + "\"";
    } else if (type == "builtin"){
	ret = "BUILTIN";
    } else {
	ret = x.in.value + "";
    }
//    ret += "[" + stringify_scope(x.bindings) + "]";
    return ret;
}

function stringify_list(x){
    if (x.in.value.cdr.type() == "nil"){
	return stringify(x.in.value.car);
    } else {
	return stringify(x.in.value.car) + " " + stringify_list(x.in.value.cdr);
    }
}