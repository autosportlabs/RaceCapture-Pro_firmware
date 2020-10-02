find -H . -not -type l -writable -name "*.[ch]" | xargs -d '\n' astyle -nQ --style=linux -s8
