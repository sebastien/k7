var ev = system.event;

try{
    ev.buffer_add();
    throw "it worked";
} catch (ex) {
    if (-1==ex.toString().search("Argument 1 of buffer_add"))
        throw "wrong exception: "+ex;
}

for(var i=0; i<1000000; i++) {
    var str = "secret"+Math.random();
    var buf = ev.buffer_new();
    ev.buffer_add(buf,str);
    var str2 = ev.buffer_remove(buf);
    if (str!=str2)
        print(str+"!="+str2+"\n");
}

