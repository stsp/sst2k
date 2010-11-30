# From this directory, distribute rge following files:
# HACKING makehelp.py sst-doc.xml sst-layer.xsl sst.xml

sst.6: sst.xml
	xmlto man sst.xml

sst-doc.txt: sst-doc.xml
	xmlto -m sst-layer.xsl txt sst-doc.xml
sst.doc: sst-doc.txt
	./makehelp.py >sst.doc

sst-doc.html: sst-doc.xml
	xmlto xhtml-nochunks sst-doc.xml

clean:
	rm -f sst.6 sst-doc.txt sst.doc
