#ifndef EXPRESSION_EVALUATOR_H
#define EXPRESSION_EVALUATOR_H

double eval(char *expression, int *err);
double eval_expression();
double eval_term();
double eval_factor();

#endif