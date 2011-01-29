var editor;

$(document).ready(
    function(){

	$.ctrl = function(key, callback, args) {
	    var isCtrl = false;
	    $(document).keydown(
		function(e) {
		    if(!args) args=[]; // IE
		    if(e.ctrlKey) isCtrl = true;
		    if(e.keyCode == key.charCodeAt(0) && isCtrl) {
			callback.apply(this, args);
			return false;
		    }
		}).keyup(
		    function(e) {
			if(e.ctrlKey) isCtrl = false;
		    });
	};

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

	editor = new CodeMirror(
	    CodeMirror.replace(textarea),
	    {
		height: "350px",
		content: textarea.value,
		path: "CodeMirror-0.91/js/",
		parserfile: ["../contrib/scheme/js/tokenizescheme.js",
			     "../contrib/scheme/js/parsescheme.js"],
		stylesheet:  "CodeMirror-0.91/contrib/scheme/css/schemecolors.css",
		autoMatchParens: true,
		lineNumbers: true,
		disableSpellcheck: true,
		markParen: function(span, good) {
		    addClass(span,
			     good ? "good-matching-paren" : "bad-matching-paren");
		},
		unmarkParen: function(span) {
		    removeClass(span, "good-matching-paren");
		    removeClass(span, "bad-matching-paren");
		}
	    });




	var m = eight.init_machine();
	function uiexecute(str){
	    var ret = eight.execute(str, m);
	    stack_trace(m);
	    $("#output").html(print(eight.stringify(m.accum)));
	}

	function uiload(str){
	    eight.load(str, m);
	    stack_trace(m);
	}

	function uistep(){

	    eight.step(m);
	    stack_trace(m);
	    if (m.paused == true) {
		$("#output").html(print(eight.stringify(m.accum)));
	    }
	}

	function uicontinue(){
	    eight.continu(m);
	    if (m.paused == true) {
		stack_trace(m);
		$("#output").html(print(eight.stringify(m.accum)));
	    }
	}

	function print(str){
	    return str.replace(/\n/g, "<br />");
	}

	function stack_trace(m){
	    $("#stacktrace").html(print(eight.stringify(m.accum)) + "<br><br><br>" +
				  print(eight.stringify(m.current_frame)));
	}



	$.ctrl('S', uistep);

	$("#exbut").click(function(){
	    uiexecute(editor.getCode());
	});
	$("#loadbut").click(function(){
	    uiload(editor.getCode());
	});
	$("#stepbut").click(function(){
	    uistep();
	});
	$("#contbut").click(function(){
	    uicontinue();
 	});

    });