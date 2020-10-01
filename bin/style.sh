find -L . -not -xtype l -writable -name "*.[ch]" | xargs -d '\n' astyle -nQ --style=linux -s8
