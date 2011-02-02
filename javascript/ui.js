var editor;
var output_string;
$(document).ready(
    function(){

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
		height: "100%",
		content: textarea.value,
		path: "CodeMirror-0.91/js/",
		parserfile: ["../../tokenizeeight.js",
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




	var m;
	function uiexecute(str){
	    var ret = eight.execute(str, m);
	    output_string("\n" + eight.stringify(m.accum) + "\n> ");
	    if (m.paused == true && m.current_frame.below) {
		stack_trace(m);
	    }
	}


	function uistep(){

	    eight.step(m);
	    stack_trace(m);
	    if (m.paused == true) {
		output_string("\n" + eight.stringify(m.accum) + "\n> ");
	    }
	}

	function uicontinue(){
	    eight.continu(m);
	    if (m.paused == true) {
		stack_trace(m);
		output_string("\n" + eight.stringify(m.accum) + "\n> ");
	    }
	}

	function print(str){
	    return str.replace(/\n/g, "<br />");
	}

	var stacktrace_showing = true;

	function stack_trace(m){
	    if (!stacktrace_showing){
		stacktrace_showing = true;
		$("#editorb").animate({width:"72.5%"}, 50);
		$("#outputb").animate({width:"72.5%"}, 50);
		$("#stacktraceb").slideDown(1000);
	    }
	    $("#stacktrace").html(print(eight.stringify(m.accum)) + "<br><br><br>" +
				  print(eight.stringify(m.current_frame)));
	}

	output_string = function(x){
	    $("#output").html($("#output").html() + print(x));
	};

	function reset(){
	    $("#output").html("&gt; ");
	    m = eight.init_machine();
	    if (stacktrace_showing){

		$("#stacktraceb").hide();

		$("#editorb").animate({width:"100%"});
		$("#outputb").animate({width:"100%"});

		stacktrace_showing = false;
		$("#stacktrace").html("");
	    };
	}


	$("#exbut").click(function(){
	    uiexecute(editor.getCode());
	});

	$("#stepbut").click(uistep);
	$("#contbut").click(uicontinue);
	$("#resetbut").click(reset);

	$("#live").keydown(function(e){
			       if ( event.which == 13 ){
				   $("#output").html($("#output").html() + $("#live").val());
				   uiexecute($("#live").val());

			       }
			   });

	$("#outputb").click(function(){
	    $("#live").focus();
	});




	editor.grabKeys(function(e){
			    if('K'.charCodeAt(0) == e.which &&
				e.ctrlKey){
				    uicontinue();
			    } else if ('E'.charCodeAt(0) == e.which &&
				       e.ctrlKey) {
				uiexecute(editor.getCode());
			    } else if ('S'.charCodeAt(0) == e.which &&
				       e.ctrlKey) {
				uistep();
			    } else if ('L'.charCodeAt(0) == e.which &&
				       e.ctrlKey) {
				reset();
			    }
			    return true;
			},
			function (k, e){
			    return (e.ctrlKey &&
				    ('K'.charCodeAt(0) == k ||
				     'E'.charCodeAt(0) == k ||
				     'S'.charCodeAt(0) == k ||
				     'L'.charCodeAt(0) == k));
			    }
		       );

	$(document).keydown(function(e){
			    if('K'.charCodeAt(0) == e.which &&
				e.ctrlKey){
				    uicontinue();
			    } else if ('E'.charCodeAt(0) == e.which &&
				       e.ctrlKey) {
				uiexecute(editor.getCode());
			    } else if ('S'.charCodeAt(0) == e.which &&
				       e.ctrlKey) {
				uistep();
			    } else if ('L'.charCodeAt(0) == e.which &&
				       e.ctrlKey) {
				reset();
			    } else {
				return;
			    }
			    e.preventDefault();
			    });
	reset();
    });