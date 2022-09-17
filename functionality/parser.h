//
// Created by m0rtis on 7/24/22.
//

#ifndef PROJEKTAUFGABE_PARSER_H
#define PROJEKTAUFGABE_PARSER_H


struct EllpackMatrix* parse_matrix(char *matrix_path);
void write_matrix(struct EllpackMatrix* matrix, char *out_path);

#endif //PROJEKTAUFGABE_PARSER_H
