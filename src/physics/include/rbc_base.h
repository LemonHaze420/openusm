#pragma once

struct rigid_body;

struct rigid_body_constraint {
    rigid_body *b1;
    rigid_body *b2;
};
