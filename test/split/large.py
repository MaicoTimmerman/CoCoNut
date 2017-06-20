from jinja2 import Template
import random

tpl = Template("""
{% for i in range(300) %}
node Node{{i}} {
    children {
        Node{{random.randint(0, 299)}} a { construct },
        Node{{random.randint(0, 299)}} b { construct },
        Node{{random.randint(0, 299)}} c { construct },
        Node{{random.randint(0, 299)}} d { construct }
    }
};

{% endfor %}
root node Noderoot {
    children {
        Node{{random.randint(0, 299)}} a { construct },
        Node{{random.randint(0, 299)}} b { construct },
        Node{{random.randint(0, 299)}} c { construct },
        Node{{random.randint(0, 299)}} d { construct }
    }
};

root phase A {
    passes {
        B
    }
};

pass B;
""")

print(tpl.render(random=random))
