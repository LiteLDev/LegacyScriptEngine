// [kit patch: LegacyScriptEngine] python-markdown compat: a block after a blank line attaches to the item at
// depth floor(indent / 4) - 1 (Python-Markdown's ListIndentProcessor), like the live MkDocs site.
import { test } from 'node:test';
import assert from 'node:assert/strict';
import { unified } from 'unified';
import remarkParse from 'remark-parse';
import { preprocess } from '../docs/node/markdown/preprocess.ts';

const md = (src) => unified().use(remarkParse).parse(preprocess(src, { compat: 'python-markdown' }));
const shape = (n) =>
  n.type === 'list' || n.type === 'listItem' || n.type === 'root'
    ? { [n.type]: n.children.map(shape) }
    : n.type === 'paragraph'
      ? 'p:' + n.children.map((c) => c.value ?? c.children?.map((x) => x.value).join('') ?? '').join('').split('\n')[0]
      : n.type;

test('6-space paragraph after a nested item belongs to the outer item (LSE Entity.teleport)', () => {
  const tree = md(
    [
      '- Parameters:',
      '    - pos : `IntPos`  ',
      '      Target position',
      '',
      '    - rot: `DirectionAngle`',
      '',
      '      (Optional) The orientation',
      '- Return value: Whether',
      '- Return value type:  `Boolean`',
    ].join('\n'),
  );
  assert.deepEqual(shape(tree), {
    root: [
      {
        list: [
          {
            listItem: [
              'p:Parameters:',
              { list: [{ listItem: ['p:pos : IntPosTarget position'] }, { listItem: ['p:rot: DirectionAngle'] }] },
              'p:(Optional) The orientation',
            ],
          },
        ],
      },
    ],
  });
});

test('items after a hoisted paragraph start a new nested list; the next hoist returns to the outer item', () => {
  const tree = md(
    [
      '- Parameters:',
      '    - a: `Integer`',
      '',
      '      (optional) A',
      '',
      '    - b: `Integer`',
      '',
      '      (optional) B',
      '',
      '- Return value: x',
    ].join('\n'),
  );
  assert.deepEqual(shape(tree), {
    root: [
      {
        list: [
          {
            listItem: [
              'p:Parameters:',
              { list: [{ listItem: ['p:a: Integer'] }] },
              'p:(optional) A',
              { list: [{ listItem: ['p:b: Integer'] }] },
              'p:(optional) B',
            ],
          },
          { listItem: ['p:Return value: x'] },
        ],
      },
    ],
  });
});

test('8-space paragraph stays in the nested item', () => {
  const tree = md(['- Parameters:', '    - rot: `DirectionAngle`', '', '        (Optional) The orientation'].join('\n'));
  assert.deepEqual(shape(tree), {
    root: [
      {
        list: [
          {
            listItem: [
              'p:Parameters:',
              { list: [{ listItem: ['p:rot: DirectionAngle', 'p:(Optional) The orientation'] }] },
            ],
          },
        ],
      },
    ],
  });
});
