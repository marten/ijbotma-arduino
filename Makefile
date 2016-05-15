Arduino-IJbema/quotes.cpp: quotes.csv quotes_gen.rb
	ruby quotes_gen.rb < $< > $@
