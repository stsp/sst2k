# Note: msgfmt.py is copied from the Python source distributionls

.SUFFIXES: .po .mo

.po.mo:
	python msgfmt.py $<

all: ru.mo

clean:
	rm *.mo
