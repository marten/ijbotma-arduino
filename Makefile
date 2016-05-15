Arduino-IJbema/quotes.cpp: quotes.txt quotes_gen.rb
	ruby quotes_gen.rb < $< > $@
