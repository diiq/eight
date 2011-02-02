var eight=new function(){function Q(b){this.value=b;this.info=[]}function p(b,c){this.bindings=[];this.inner=new Q(b);this.inner.info.type=c}function k(b,c,d){this.next=b;this.instruction=c;this.flag=d}function w(b,c){this.below=b;this.next=o;this.scope=[];this.rib=[];this.signal_handler=null;this.trace=c}function R(){this.current_frame=this.base_frame=new w(null,i("base-frame"));this.accum=null;this.paused=true}function n(b){return b.type()=="nil"}function y(b,c){this.car=b;this.cdr=c}function E(b,
c){return new p(new y(b,c),"cons")}function i(b){return new p(b,"symbol")}function x(b,c){var d=b.copy(),e=c.copy();if(n(b)){var j=E(o,e);j.bindings=e.bindings;e.bindings=[];return j}if(n(c)){j=E(d,o);j.bindings=d.bindings;d.bindings=[];return j}d.bindings=[];e.bindings=[];var s=j=E(d,e),r=b.bindings,z=c.bindings;d=d.bindings;e=e.bindings;var A=[],l;for(l in r)if(l in z)if(r[l]==z[l])A[l]=r[l];else{d[l]=r[l];e[l]=r[l]}else A[l]=r[l];for(l in z)l in r||(A[l]=z[l]);s.bindings=A;return j}function v(b){if(n(b))return o;
return b.inner.value.car}function F(b,c){var d=[],e;for(e in b)d[e]=b[e];for(e in c)L(c[e])||(d[e]=c[e]);return d}function f(b){if(n(b))return o;b.inner.value instanceof y||alert("Car of a thing not consed!"+t(b));var c=v(b).copy();c.bindings=F(b.bindings,c.bindings);return c}function h(b){if(n(b))return o;b.inner.value instanceof y||alert("Cdr of a thing not consed!");var c;c=n(b)?o:b.inner.value.cdr;c=c.copy();c.bindings=F(b.bindings,c.bindings);return c}function g(){for(var b=o,c=arguments.length-
1;c>=0;c--)b=x(arguments[c],b);return b}function B(b,c){if(n(b))return c;return x(f(b),B(h(b),c))}function L(b){if(b&&b.type()=="cons"&&v(b).type()=="symbol"&&v(b).inner.value=="leak")return true;return false}function C(b,c){var d;d=b.inner.value in b.bindings?b.bindings[b.inner.value]:g(i("leak"));for(var e=c.current_frame;L(d);){d=e.scope[b.inner.value];e=e.next}if(d==undefined)d=c.base_frame.scope[b.inner.value];if(d==undefined)d=o;return d}function D(b,c,d){if(b.type()=="nil")return d;var e=f(c),
j;j=e.type()=="cons"&&v(e).type()=="symbol"&&v(e).inner.value=="*"?true:false;if(j){b=new k(d,g(b,h(c)),"asterpend_continue");b=new k(b,f(h(e)),"evaluate")}else{j=e.type()=="cons"&&v(e).type()=="symbol"&&v(e).inner.value=="@"?true:false;if(j){b=new k(d,g(b,h(c)),"atpend_continue");b=new k(b,f(h(e)),"evaluate")}else{var s=f(b),r="argument";if(s.type()=="symbol"&&s.inner.value=="..."){if(n(c))return d;s=f(h(b));b=x(o,b);r="e_argument"}if(s.type()=="cons"){j=f(h(s));e=e.type()=="cons"&&f(e).type()==
"symbol"&&f(e).inner.value==","?f(h(e)):g(f(s),e)}else{j=s;e=e}b=new k(d,g(h(b),h(c)),"continue");b=new k(new k(b,j,r),e,"evaluate")}}return b}function M(b){if(n(b))return o;return x(g(i("clear"),f(b)),M(h(b)))}function G(b){var c=b.current_frame.next;if(c==null)if(b.current_frame.below!=null)b.current_frame=b.current_frame.below;else b.paused=true;else{b.current_frame.next=c.next;if(c.flag=="evaluate")if(c.instruction.type()=="builtin")c.instruction.inner.value(b);else if(c.instruction.type()=="symbol"){var d=
C(c.instruction,b);n(d)&&alert("thass no' a symbol, lassie: "+t(c.instruction));b.accum=f(d)}else{a:{for(d in c.instruction.bindings){d=false;break a}d=true}if(d)if(c.instruction.type()=="cons")if(f(c.instruction).type()=="symbol"&&f(c.instruction).inner.value=="clear")b.accum=f(h(c.instruction));else{b.current_frame=new w(b.current_frame,c.instruction);b.current_frame.scope=b.current_frame.below.scope;b.current_frame.next=new k(new k(b.current_frame.next,h(c.instruction),"apply"),f(c.instruction),
"evaluate")}else b.accum=c.instruction;else{b.current_frame.scope=F(b.current_frame.scope,c.instruction.bindings);c=c.instruction.copy();c.bindings=[];b.current_frame.next=new k(b.current_frame.next,c,"evaluate")}}else if(c.flag=="apply"){b.current_frame.next=new k(b.current_frame.next,h(b.accum),"do");b.current_frame.next=D(f(b.accum),c.instruction,b.current_frame.next)}else if(c.flag=="continue")b.current_frame.next=D(f(c.instruction),f(h(c.instruction)),b.current_frame.next);else if(c.flag=="asterpend_continue")b.current_frame.next=
D(f(c.instruction),B(b.accum,f(h(c.instruction))),b.current_frame.next);else if(c.flag=="atpend_continue")b.current_frame.next=D(f(c.instruction),B(M(b.accum),f(h(c.instruction))),b.current_frame.next);else if(c.flag=="argument")b.current_frame.rib[c.instruction.inner.value]=g(b.accum);else if(c.flag=="e_argument"){c=c.instruction.inner.value;if(c in b.current_frame.rib){d=b.current_frame.rib[c];d=B(f(d),g(b.accum));b.current_frame.rib[c]=g(d)}else b.current_frame.rib[c]=g(g(b.accum))}else if(c.flag==
"do"){b.current_frame.scope=b.current_frame.rib;d=b.current_frame.next;for(var e=b.current_frame;!n(c.instruction);){e.next=new k(b.current_frame.next,f(c.instruction),"evaluate");e=e.next;c.instruction=h(c.instruction)}e.next=d}}}function q(b,c,d,e){d=g(u(H(d)),new p(e,"builtin"));d.inner.info["function-name"]=i(c);b.base_frame.scope[c]=g(d)}function m(b,c){return f(C(i(b),c))}function S(b){q(b,"cons","(car cdr)",function(c){var d=m("cdr",c);!(d.inner.value instanceof y)&&!n(d)&&alert("Warning that you should make real lists!");
c.accum=x(m("car",c),d)});q(b,"car","(x)",function(c){var d=m("x",c);c.accum=f(d)});q(b,"cdr","(x)",function(c){var d=m("x",c);c.accum=h(d)});q(b,"info","(x)",function(c){var d=m("x",c);c.accum=new p(d.inner.info,"table")});q(b,"break","()",function(c){c.paused=true});q(b,"'","((clear x))",function(c){var d=m("x",c),e=c.current_frame;c.current_frame=e.below;d.bindings=[];I(d,d.bindings,c);c.accum=d;c.current_frame=e});q(b,"set!","('symbol value)",function(c){var d=m("symbol",c),e=m("value",c),j=C(d,
c);if(n(j))c.base_frame.scope[d.inner.value]=g(e);else j.inner.value.car=e;c.accum=e});q(b,"leak","(symbol expression)",function(c){var d=m("symbol",c),e=m("expression",c);e.bindings[d.inner.value]=g(i("leak"));c.accum=e});q(b,"atom-p","(x)",function(c){var d=m("x",c);c.accum=d.type()=="cons"?o:i("t")});q(b,"oif","(test 'then 'else)",function(c){var d=m("test",c);c.current_frame.next=n(d)?new k(c.current_frame.next,m("else",c),"evaluate"):new k(c.current_frame.next,m("then",c),"evaluate")});q(b,"is",
"(a b)",function(c){var d=m("a",c),e=m("b",c);c.accum=d==e||d.type()==e.type()&&d.inner.value===e.inner.value?i("t"):o})}function I(b,c,d){if(b.type()=="symbol"&&!(b.inner.value in c)){d=C(b,d);n(d)||(c[b.inner.value]=d)}else if(b.type()=="cons"){I(f(b),c,d);I(h(b),c,d)}}function J(b){return u(H("((clear (() "+b+")))"))}function H(b){b=b.split(/\"/);var c=[];for(a in b)if(a%2==0)c=c.concat(T(b[a]));else c.push('"'+b[a]);return c.reverse()}function T(b){b=b.replace(/\#.*\n/g," ");b=b.replace(/\'\(/g,
" '[ ");b=b.replace(/\@\(/g," @[ ");b=b.replace(/\*\(/g," *[ ");b=b.replace(/\,\(/g," ,[ ");b=b.replace(/\(/g," ( ");b=b.replace(/\)/g," ) ");b=b.replace(/\'\[/g," '( ");b=b.replace(/\@\[/g," @( ");b=b.replace(/\*\[/g," *( ");b=b.replace(/\,\[/g," ,( ");b=b.split(/\s+/);var c=[],d=0;for(token in b)if(b[token]){c[d]=b[token];d++}return c}function u(b){var c=b.pop();if(c=="(")return U(b);else if(c[0]=="'"){b.push(c.substr(1,c.length));return g(i("'"),u(b))}else if(c[0]=="@"){b.push(c.substr(1,c.length));
return g(i("@"),u(b))}else if(c[0]=="*"){b.push(c.substr(1,c.length));return g(i("*"),u(b))}else if(c[0]==","){b.push(c.substr(1,c.length));return g(i(","),u(b))}else{if(c[0]=='"'){b=c.substr(1,c.length);b=new p(b,"string")}else b=c.match(/^\-?\d+\.?\d*?$/)?new p(parseFloat(c),"number"):i(c);return b}}function U(b){for(var c=[];b[b.length-1]!=")";)c.push(u(b));b.pop();return g.apply(this,c)}function t(b){if(b)if(b instanceof p){var c=b.type();return c=="cons"?"("+N(b)+")":c=="nil"?"()":c=="string"?
'"'+b.inner.value+'"':c=="builtin"?"BUILTIN":b.inner.value+""}else if(b instanceof k)return K(b);else if(b instanceof w)return O(b);else{alert("I don't know how to stringiy that.");return"STRINGIFY ERR"}else return"NULL"}function O(b){return b.below?t(b.trace)+"\n scoped as "+P(b.scope)+"\n ribbed as "+P(b.rib)+"\n performing:\n"+K(b.next)+"\n\n"+O(b.below):""}function P(b){var c="",d;for(d in b)c+="\n"+d+" -> "+t(b[d]);return c}function K(b){return b==null?"":"|-> "+t(b.instruction)+", flagged "+
b.flag+"\n"+K(b.next)}function N(b){return b.inner.value.cdr.type()=="nil"?t(b.inner.value.car):t(b.inner.value.car)+" "+N(b.inner.value.cdr)}p.prototype.type=function(){return this.inner.info.type};p.prototype.copy=function(){var b=new p(this.inner.value,this.type());b.inner.info=this.inner.info;b.bindings=this.bindings;return b};var o=new p(null,"nil");this.continu=function(b){for(b.paused=false;b.paused==false;)G(b)};this.step=function(b){b.paused=false;G(b)};this.load=function(b,c){var d=J(b);
d=new k(null,d,"evaluate");c.current_frame=new w(c.base_frame,i("initial"));c.current_frame.next=d;c.paused=false};this.execute=function(b,c){var d=new k(null,J(b),"evaluate");c.current_frame=new w(c.base_frame,i("initial"));c.current_frame.next=d;for(c.paused=false;c.paused==false;)G(c);return c.accum};this.init_machine=function(){var b=new R;b.base_frame.scope.q=g(i("sq"));b.base_frame.scope.r=g(i("sr"));b.base_frame.scope.s=g(i("ss"));S(b);return b};this.stringify=t;this.parse=u;this.preparse=
H;this.ultraparse=J};