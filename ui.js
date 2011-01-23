var editor;

$(document).ready(function(){

    function addClass(element, className) {
	if (!editor.win.hasClass(element, className)) {
	    element.className = ((element.className.split(" ")).concat([className])).join(" ");
	}
    }

    function removeClass(element, className) {
	if (editor.win.hasClass(element, className)) {
	    var classes = element.className.split(" ");
	    for (var i = classes.length - 1 ; i >= 0; i--) {
		if (classes[i] === className) {
		    classes.splice(i, 1);
		}
	    }
	    element.className = classes.join(" ");
	}
    }

    var textarea = document.getElementById('code');
    editor = new CodeMirror(CodeMirror.replace(textarea), {
	height: "350px",
	content: textarea.value,
	path: "CodeMirror-0.91/js/",
	parserfile: ["../contrib/scheme/js/tokenizescheme.js",
    "../contrib/scheme/js/parsescheme.js"],
	stylesheet:  "CodeMirror-0.91/contrib/scheme/css/schemecolors.css",
	autoMatchParens: true,
	lineNumbers: true,
	disableSpellcheck: true,
	markParen: function(span, good) {addClass(span, good ? "good-matching-paren" : "bad-matching-paren");},
	unmarkParen: function(span) {removeClass(span, "good-matching-paren"); removeClass(span, "bad-matching-paren");}
    });
 });

var m;
function execute(str){
    var lamda = parse(preparse("((clear ((a b c) a)) q r s)"));
    m = new Machine();
    m.base_frame.scope["q"] = list(symbol("sq"));
    m.base_frame.scope["r"] = list(symbol("sr"));
    m.base_frame.scope["s"] = list(symbol("ss"));
    var op = new Operation(null, lamda, "evaluate");
    m.current_frame = new Frame(m.base_frame, "initial");
    m.current_frame.next = op;
    stack_trace(m);
}

function step(){
    machine_step(m);
    stack_trace(m);
}

function print(str){
    return str.replace(/\n/g, "<br />");
}
function stack_trace(m){
    $("#stacktrace").html(print(stringify(m.accum)) + "<br><br><br>" +
			  print(stringify(m.current_frame)));
}